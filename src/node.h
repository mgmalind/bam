
/***** dependency graph *******/

#include <time.h>
#include "tree.h"

/**/
struct DEPENDENCY
{
	struct NODE *node;
	struct DEPENDENCY *next;
};

struct SCANNER
{
	struct SCANNER *next;
	int (*scannerfunc)(struct NODE *, struct SCANNER *info);
};

/**/
struct NODE
{
	/* *** */
	struct GRAPH *graph;
	struct NODE *next;
	struct DEPENDENCY *firstdep; /* list of dependencies */

	RB_ENTRY(NODE) rbentry;

	/* filename and the tool to build the resource */
	unsigned filename_len; /* including zero term */
	char *filename; /* this contains the filename with the FULLPATH */
	
	char *cmdline; /* command line that should be executed to build this node */
	char *label; /* what to print when we build this node */
	
	struct SCANNER *firstscanner;

	unsigned hashid;
	
	time_t timestamp;
	unsigned id; /* used when doing traversal with marking (bitarray) */
	
	/* various flags */
	unsigned dirty:1; /* set if the node has to be rebuilt */
	unsigned depchecked:1; /* set if a dependency checker have processed the file */
	unsigned cached:1;
	unsigned parenthastool:1; /* set if a parent has a tool */
	unsigned counted:1;
	
	volatile unsigned workstatus:2; /* 0 = undone, 1 = in the workings, 2 = done*/
};

/* cache node */
struct CACHENODE
{
	RB_ENTRY(CACHENODE) rbentry;

	unsigned hashid;
	time_t timestamp;
	char *filename;
	
	unsigned deps_num;
	unsigned *deps; /* index id, not hashid */
};


struct HEAP;
struct GRAPH;
struct CONTEXT;

/* node status */
#define NODESTATUS_UNDONE 0
#define NODESTATUS_WORKING 1
#define NODESTATUS_DONE 2

/* node creation error codes */
#define NODECREATE_OK 0
#define NODECREATE_EXISTS 1
#define NODECREATE_NOTNICE 2

/* node walk flags */
#define NODEWALK_FORCE 1
#define NODEWALK_TOPDOWN 2
#define NODEWALK_BOTTOMUP 4
#define NODEWALK_UNDONE 8
#define NODEWALK_QUICK 16

/* you destroy graphs by destroying the heap */
struct GRAPH *node_create_graph(struct HEAP *heap);
struct HEAP *node_graph_heap(struct GRAPH *graph);

/* cache */
int node_cache_save(const char *filename, struct GRAPH *graph);
struct CACHE *node_cache_load(const char *filename);
struct CACHENODE *node_cache_find_byhash(struct CACHE *cache, unsigned hashid);
struct CACHENODE *node_cache_find_byindex(struct CACHE *cache, unsigned index);
int node_cache_do_dependency(struct CONTEXT *context, struct NODE *node);

/* */
int node_create(struct NODE **node, struct GRAPH *graph, const char *filename, const char *label, const char *cmdline);
struct NODE *node_find(struct GRAPH *graph, const char *filename);
struct NODE *node_add_dependency(struct NODE *node, const char *filename);

struct NODEWALKPATH
{
	struct NODEWALKPATH *parent;
	struct NODE *node;
};

struct NODEWALK
{
	struct NODE *node;
	void *user;
	
	struct NODEWALKPATH *parent;
	
	unsigned depth;
	int flags;
	int (*callback)(struct NODEWALK *);
	unsigned char *mark;
};

int node_walk(
	struct NODE *node,
	int flags,
	int (*callback)(struct NODEWALK *info),
	void *u);

void node_debug_dump(struct GRAPH *graph);
void node_debug_dump_tree(struct NODE *root);
void node_debug_dump_dot(struct GRAPH *graph, struct NODE *node);
