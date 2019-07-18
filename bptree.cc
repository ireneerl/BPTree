#include "bptree.h"
#include <vector>
#include <sys/time.h>
#include <stdbool.h>
int Size_data;
vector<uint> Key_vector;

void
print_performance(struct timeval begin, struct timeval end)
{
	long diff = (end.tv_sec - begin.tv_sec) * 1000 * 1000 + (end.tv_usec - begin.tv_usec);
	printf("%10.0f req/sec (lat:%7ld usec)\n", ((double)Size_data) / ((double)diff/1000.0/1000.0), diff);
}

struct timeval
cur_time(void)
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return t;
}

void
init_vector(void)
{
	for (int i = 0; i < Size_data; i++) {
    int key = rand() % 1000000;
		Key_vector.push_back(key);
	}
}

void
print_tree_core(NODE *n)
{
	printf("[");
	for (int i = 0; i < n->nkey; i++) {
		if (!n->isLeaf) {
			print_tree_core(n->chi[i]);
		}
		printf("%d",n->key[i]);
		// printf("%d -> %p, %p, %d, %p", i, (void *)n, (void *)n->parent, n->key[i],(void *)n->chi[i]);
		if (i != n->nkey-1 && n->isLeaf) putchar(' ');
	}
	if (!n->isLeaf) {
		print_tree_core(n->chi[n->nkey]);
	}
	printf("]");
}

void
print_tree(NODE *node)
{
	print_tree_core(node);
	printf("\n"); fflush(stdout);
}

NODE *
alloc_leaf(NODE *parent)
{
	NODE *node;
	if (!(node = (NODE *)calloc(1, sizeof(NODE)))) ERR;
	node->isLeaf = true;
	node->parent = parent;
	node->nkey = 0;

	return node;
}

NODE *
alloc_internal(NODE *parent)
{
	NODE *node;
	if (!(node = (NODE *)calloc(1, sizeof(NODE)))) ERR;
	node->isLeaf = false;
	node->parent = parent;
	node->nkey = 0; /* CodeQuiz-2 */

	return node;
}

NODE *
alloc_root(NODE *left, int rs_key, NODE *right)
{
	NODE *node;

	if (!(node = (NODE *)calloc(1, sizeof(NODE)))) ERR;
	node->parent = NULL;
	node->isLeaf = false;
	node->key[0] = rs_key;
	node->chi[0] = left;
	node->chi[1] = right;
	node->nkey = 1;

	return node;
}

NODE *
find_leaf(NODE *node, int key)
{
	int kid;

	if (node->isLeaf) return node;
	for (kid = 0; kid < node->nkey; kid++) {
		if (key < node->key[kid]) break;
	}

	return find_leaf(node->chi[kid], key);
}

NODE *
insert_in_leaf(NODE *leaf, int key, DATA *data)
{
	int i;
	if (key < leaf->key[0]) {
		for (i = leaf->nkey; i > 0; i--) {
			leaf->key[i] = leaf->key[i-1] ;
		}
		for (i = leaf->nkey+1; i > 0; i--) {
			leaf->chi[i] = leaf->chi[i-1] ;
    }
		leaf->key[0] = key;
		leaf->chi[0] = (NODE *)data;
	}
	else {
		for (i = 0; i < leaf->nkey; i++) {
			if (key < leaf->key[i]) break;
		}
		for (int j = leaf->nkey; j > i; j--) {
			leaf->key[j] = leaf->key[j-1] ;
		}
		for (int j = leaf->nkey+1; j > 0; j--) {
			leaf->chi[j] = leaf->chi[j-1] ;
    }

		leaf->key[i] = key;
		leaf->chi[i] = (NODE *)data;
	}
	leaf->nkey++;

	return leaf;
}

void
insert_in_temp(TEMP *temp, int key, void *ptr)
{
	int i;
	if (key < temp->key[0]) {
		for (i = temp->nkey; i > 0; i--) {
			temp->chi[i] = temp->chi[i-1] ;
			temp->key[i] = temp->key[i-1] ;
		}
		temp->key[0] = key;
		temp->chi[0] = (NODE *)ptr;
	}
	else {
		for (i = 0; i < temp->nkey; i++) {
			if (key < temp->key[i]) break;
		}
		for (int j = temp->nkey; j > i; j--) {
			temp->chi[j] = temp->chi[j-1] ;
			temp->key[j] = temp->key[j-1] ;
		}
		temp->key[i] = key;
		temp->chi[i] = (NODE *)ptr;
	}

	temp->nkey++;
}

void
erase_entries(NODE *node)
{
	for (int i = 0; i < N-1; i++) node->key[i] = 0;
	for (int i = 0; i < N; i++) node->chi[i] = NULL;
	node->nkey = 0;
}

void
copy_from_temp_to_left(TEMP temp, NODE *left)
{
	for (int i = 0; i < (int)ceil(N/2); i++) {
		left->key[i] = temp.key[i];
		left->chi[i] = temp.chi[i];
		left->nkey++;
	}
}

void
copy_from_temp_to_right(TEMP temp, NODE *right)
{
	for (int i = (int)ceil(N/2); i < N; i++) {
		right->key[i - (int)ceil(N/2)] = temp.key[i];
		right->chi[i - (int)ceil(N/2)] = temp.chi[i];
		right->nkey++;
	}
}

void
copy_from_temp_to_left_parent(TEMP *temp, NODE *left)
{
	for (int i = 0; i < (int)ceil((N+1)/2); i++) {
		left->key[i] = temp->key[i];
		left->chi[i] = temp->chi[i];
		left->nkey++;
	}
	left->chi[(int)ceil((N+1)/2)] = temp->chi[(int)ceil((N+1)/2)];
}

void
copy_from_temp_to_right_parent(TEMP *temp, NODE *right)
{
	int id;

	for (id = ((int)ceil((N+1)/2) + 1); id < N; id++) {
		right->chi[id - ((int)ceil((N+1)/2) + 1)] = temp->chi[id];
		right->key[id - ((int)ceil((N+1)/2) + 1)] = temp->key[id];
		right->nkey++;
	}
	right->chi[id - ((int)ceil((N+1)/2) + 1)] = temp->chi[id];

	for (int i = 0; i < right->nkey+1; i++) right->chi[i]->parent = right;
}

void
copy_from_left_to_temp(TEMP *temp, NODE *left)
{
	int i;
	bzero(temp, sizeof(TEMP));
	for (i = 0; i < (N-1); i++) {
		temp->chi[i] = left->chi[i];
		temp->key[i] = left->key[i];
	} temp->nkey = N-1;
	temp->chi[i] = left->chi[i];
}

void
insert_after_left_child(NODE *parent, NODE *left_child, int rs_key, NODE *right_child)
{
	int lcid = 0;
	int rcid = 0; // right_child_id
	int i;

	for (i = 0; i < parent->nkey+1; i++) {
		if (parent->chi[i] == left_child) {
			lcid = i; // left_child_id
			rcid = lcid+1; break;
		}
	}
	assert(i != parent->nkey+1);

	for (i = parent->nkey+1; i > rcid; i--) parent->chi[i] = parent->chi[i-1];
	for (i = parent->nkey; i > lcid; i--) parent->key[i] = parent->key[i-1];

	parent->key[lcid] = rs_key;
	parent->chi[rcid] = right_child;
	parent->nkey++;
}

void
insert_temp_after_left_child(TEMP *temp, NODE *left_child, int rs_key, NODE *right_child)
{
	int lcid = 0;
	int rcid = 0; // right_child_id
	int i;

	for (i = 0; i < temp->nkey+1; i++) {
		if (temp->chi[i] == left_child) {
			lcid = i; // left_child_id
			rcid = lcid+1; break;
		}
	} assert(i != temp->nkey+1);

	for (i = temp->nkey+1; i > rcid; i--) temp->chi[i] = temp->chi[i-1];
	for (i = temp->nkey; i > lcid; i--) temp->key[i] = temp->key[i-1];

	temp->key[lcid] = rs_key;
	temp->chi[rcid] = right_child;
	temp->nkey++;
}

void
print_temp(TEMP t)
{
	int i;

	for (i = 0; i < t.nkey; i++) {
		printf("[%p]", t.chi[i]);
		printf("%d", t.key[i]);
	}
	printf("[%p]\n", t.chi[i]);
}

void
insert_in_parent(NODE *left_child, int rs_key, NODE *right_child)
{
	NODE *left_parent;
	NODE *right_parent;

	if (left_child == Root) {
		Root = alloc_root(left_child, rs_key, right_child);
		left_child->parent = right_child->parent = Root;
		return;
	}
	left_parent = left_child->parent;
	if (left_parent->nkey < N-1) {
		insert_after_left_child(left_parent, left_child, rs_key, right_child);
	}
	else {// split
		TEMP temp;
		copy_from_left_to_temp(&temp, left_parent);
		insert_temp_after_left_child(&temp, left_child, rs_key, right_child);

		erase_entries(left_parent);
		right_parent = alloc_internal(left_parent->parent);
		copy_from_temp_to_left_parent(&temp, left_parent);
		int rs_key_parent = temp.key[(int)ceil(N/2)];
		copy_from_temp_to_right_parent(&temp, right_parent);
		insert_in_parent(left_parent, rs_key_parent, right_parent);
	}
}

void
insert(int key, DATA *data)
{
	NODE *leaf;

	if (Root == NULL) {
		leaf = alloc_leaf(NULL);
		Root = leaf;
	}
	else {
		leaf = find_leaf(Root, key);
	}

	if (leaf->nkey < (N-1)) {
		insert_in_leaf(leaf, key, data);
	}
	else { // split in leaf
		NODE *left = leaf;
		NODE *right = alloc_leaf(leaf->parent);
		TEMP temp;
    NODE *right_plusonepointer = left->chi[left->nkey];

		copy_from_left_to_temp(&temp, left);
		insert_in_temp(&temp, key, data);

		// right->chi[N-1] = left->chi[N-1];
		// left->chi[N-1] = right;

		erase_entries(left);
		copy_from_temp_to_left(temp, left);
		copy_from_temp_to_right(temp, right);
		right->chi[right->nkey] = right_plusonepointer;
		left->chi[left->nkey] = right;

		int rs_key = right->key[0]; // right smallest key
		insert_in_parent(left, rs_key, right);
	}
}

void
init_root(void)
{
	Root = NULL;
}

void find_range(int key, int keyend){
	int i, newkey=0;
	NODE *n = find_leaf(Root, key);
	if (n == NULL) return;

	for (i = 0; i < n->nkey && n->key[i] < key; i++) ;
	for (; i < n->nkey && n->key[i] <= keyend; i++) {
		printf("%d -> %p\n", n->key[i], (void *)n->parent);
	}
	n = n->parent;
	if (!n->isLeaf) {
		newkey = n->key[0];
		if(n->chi[n->nkey]!=NULL){
			if(key != n->key[n->nkey-1] && newkey <= key) newkey = n->key[n->nkey-1];
		}
		if(newkey < key){
			n = n->parent;
			newkey = n->key[0];
		}
	}
	if (key != newkey)find_range(newkey, keyend);
}


void find_range_2(int keystart, int keyend){
	int i;
		NODE *n = find_leaf(Root, keystart);
		if (n == NULL) return;
		for (i = 0; i < n->nkey && n->key[i] < keystart; i++) ;
		if (i == n->nkey) return;
		while (n != NULL) {
			for (; i < n->nkey && n->key[i] <= keyend; i++) {
				printf("%d ", n->key[i]);
			}
			n = n->chi[n->nkey];
			i = 0;
		}
}


void
search_core(const int key)
{
	NODE *n = find_leaf(Root, key);
	for (int i = 0; i < n->nkey+1; i++) {
		if (n->key[i] == key) return;
	}
  cout << "Key not found: " << key << endl;
	ERR;
}

void
search_single(void)
{
	for (int i = 0; i < (int)Key_vector.size(); i++) {
		search_core(Key_vector[i]);
  }
}

int
interactive(int key)
{
  std::cout << "Key start: ";
  std::cin >> key;
	return key;

}

int
main(int argc, char *argv[])
{
  Size_data = 10;
	int keys[10] = {4,7,45,31,5,47,12,54,23,25};
  struct timeval begin, end;

	if (argc == 2) Size_data = atoi(argv[1]);
	init_vector();
	init_root();

	printf("-----Insert-----\n");
	begin = cur_time();
	for (int i = 0; i < Size_data; i++) {
		insert(keys[i], NULL);
		// insert(i, NULL); //by looping
    print_tree(Root);
  }
	end = cur_time();
	//print_performance(begin, end);


	printf("----Search (%d)-----\n", 1);
  // search_single(interactive());
	int keystart, keyend;
	std::cout << "Key start: ";
  std::cin >> keystart;
	std::cout << "Key end: ";
	std::cin >> keyend;

	begin = cur_time();
	// find_range(keystart, keyend);
	find_range_2(keystart, keyend);
	end = cur_time();
	print_performance(begin, end);


	return 0;
}
