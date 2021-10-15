#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/linkage.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");

#define BUF_LEN 100	// length for read/write buffer

static struct proc_dir_entry* proc_entry;	// pointer to proc entry
static char msg[BUF_LEN];	// buffer to store read/write messages
static int procfs_buf_len;	// variable to hold length of message
int len;
ssize_t size;
static int count;		// run count

extern long (*STUB_start_elevator)(void);	// start_elevator function object
extern long (*STUB_issue_request)(int,int,int);		// issue request function object
extern long (*STUB_stop_elevator)(void);	// stop elevator function object

// struct for a worker object
typedef struct worker{
	int type;	// type of worker
	int weight;	// total weight of worker including tools
	int destination_floor;	// the destination floor
	struct list_head list;	// the floor the worker is waiting on
}Worker;

// struct for an incoming request(m)
typedef struct request{
	int start;	// starting floor
	int dest;	// destination floor
	int type;	// type of worker
}Request;
// data pointer for kthread
Request *reqPtr = NULL;

struct task_struct *service_thread;	// pointer to request thread
struct task_struct *runElevator_thread;		// pointer to start elevator thread

// list for each floor
struct list_head floor1;
struct list_head floor2;
struct list_head floor3;
struct list_head floor4;
struct list_head floor5;
struct list_head floor6;
struct list_head floor7;
struct list_head floor8;
struct list_head floor9;
struct list_head floor10;
struct list_head elevator;

// shared data
static int isRunning = 0;	// flag to indicate current elevator running status
static char *state;		// the state of the elevator
static int floor;		// the current floor the elvator is on
static int load;		// the current weight of the elevator
static int status[3][10] = { {0,0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0} };	// status of each floor for each worker(m)
static int passengers;		// the total number of passengers
static int waiting;		// the number of passengers waiting
static int serviced;		// the total number of passengers serviced

int run_elevator(void *data){
	while(isRunning == 1){

	}
	return 0;
}

int service_request(void *arg){
	// flags for what kind of worker it is
	bool daily = false;
	bool maintenance = false;
	bool mail = false;
	int start_floor = reqPtr->start; 	// worker starting floor

	// creating a worker object and allocating space 
	Worker* worker;
	worker = kmalloc(sizeof(Worker),__GFP_RECLAIM);
	worker->type = reqPtr->type;
	worker->destination_floor = reqPtr->dest;
	if(worker->type == 1){
		worker->weight = 170;
		maintenance = true;
	}
	else if(worker->type == 2){
		worker->weight = 225;
		mail = true;
	}
	else{
		worker->weight = 150;
		daily = true;
	}
	// adding the worker to the list of whatever floor they are waiting on
	switch (start_floor){
		case 1:
			list_add_tail(&worker->list, &floor1 );
			if(daily)
				status[0][0] += 1;
			else if(maintenance)
				status[1][0] += 1;
			else
				status[2][0] += 1;
				break;
		case 2:
			list_add_tail(&worker->list, &floor2 );
			if(daily)
				status[0][1] += 1;
			else if(maintenance)
				status[1][1] += 1;
			else
				status[2][1] += 1;
			break;
		case 3:
			list_add_tail(&worker->list, &floor3 );
			if(daily)
				status[0][2] += 1;
			else if(maintenance)
				status[1][2] += 1;
			else
				status[2][2] += 1;
			break;
		case 4:
				list_add_tail(&worker->list, &floor4 );
			if(daily)
				status[0][3] += 1;
			else if(maintenance)
				status[1][3] += 1;
			else
				status[2][3] += 1;
			break;
		case 5:
			list_add_tail(&worker->list, &floor5 );
			if(daily)
				status[0][4] += 1;
			else if(maintenance)
				status[1][4] += 1;
			else
				status[2][4] += 1;
			break;
		case 6:
			list_add_tail(&worker->list, &floor6 );
			if(daily)
				status[0][5] += 1;
			else if(maintenance)
				status[1][5] += 1;
			else
				status[2][5] += 1;
			break;
		case 7:
			list_add_tail(&worker->list, &floor7 );
			if(daily)
				status[0][6] += 1;
			else if(maintenance)
				status[1][6] += 1;
			else
				status[2][6] += 1;
			break;
		case 8:
			list_add_tail(&worker->list, &floor8 );
			if(daily)
				status[0][7] += 1;
			else if(maintenance)
				status[1][7] += 1;
			else
				status[2][7] += 1;
			break;
		case 9:
			list_add_tail(&worker->list, &floor9 );
			if(daily)
				status[0][8] += 1;
			else if(maintenance)
				status[1][8] += 1;
			else
				status[2][8] += 1;
			break;
		case 10:
			list_add_tail(&worker->list, &floor10 );
			if(daily)
				status[0][9] += 1;
			else if(maintenance)
				status[1][9] += 1;
			else
				status[2][9] += 1;
			break;
	}
	waiting += 1;	// increase total number of waiting passengers
	return 0;
}

long start_elevator(void) {
	printk(KERN_NOTICE "%s\n", __FUNCTION__);
	if(isRunning == 1){
		printk(KERN_WARNING "Elevator module already running\n");
		return 1;
	}
	// setting shared data
	else if(isRunning == 0){
		isRunning = 1;
		strcpy(state, "IDLE");
		floor = 1;
		load = 0;
		passengers = 0;
		waiting = 0;
		serviced = 0;
		count = 0;
		runElevator_thread = kthread_run(run_elevator, NULL, "Run Elevator");	// intitializing pointer to run elevator thread
		return 0;
	}
	else{
		printk(KERN_ERR "Could not load elevator module\n");
	}


	return isRunning;
}

long issue_request(int start_floor, int destination_floor, int type){

	// creating the worker object
	if(start_floor >= 0 && start_floor <= 0 && destination_floor >= 0 && destination_floor <= 10 && type >=0 && type <= 10 && isRunning == 1){
		// setting the request struct with the current request
		reqPtr->start = start_floor;
		reqPtr->dest = destination_floor;
		reqPtr->type = type;

		service_thread = kthread_run(service_request, reqPtr, "Elevator Request");	// initializing pointer to request thread
		return 0;
	}
	else{
		if (isRunning == 1){
			printk(KERN_ERR "Elevator not running, start elevator to issue request");
		}
		else{
			printk(KERN_ERR "Invalid request input");
		}
		return -1;
	}
}

long stop_elevator(void){
	if(load == 0){
		isRunning = 0;
		strcpy(state, "OFFLINE");
		floor = 1;
		return 0;
	}
	else 
		return 1;
}

// procfile read (m)
ssize_t procfile_read(struct file* file, char* ubuf, size_t count, loff_t* ppos){
	char* temp = kmalloc(sizeof(char)*50, GFP_KERNEL);
	sprintf(temp, "Elevator state: %s\n", state);
	sprintf(temp, "Current floor: %d\n", floor);
	sprintf(temp, "Current weight: %d\n", load);
	sprintf(temp, "Elevator stutus: (not yet implemented)\n");
	sprintf(temp, "Number of passengers: %d\n", passengers);
	sprintf(temp, "Number of passengers waiting: %d\n", waiting);
	sprintf(temp, "Number of passengers serviced: %d\n\n", serviced);
	if (floor == 10)
		sprintf(temp, "[*] Floor 10: (not yet implemented)\n");
	else 
		sprintf(temp, "[ ] Floor 10: (not yet implemented)\n");
	if (floor == 9)
		sprintf(temp, "[*] Floor 9: (not yet implemented)\n");
	else 
		sprintf(temp, "[ ] Floor 9: (not yet implemented)\n");
	if (floor == 8)
		sprintf(temp, "[*] Floor 8: (not yet implemented)\n");
	else 
		sprintf(temp, "[ ] Floor 8: (not yet implemented)\n");
	if (floor == 7)
		sprintf(temp, "[*] Floor 7: (not yet implemented)\n");
	else 
		sprintf(temp, "[ ] Floor 7: (not yet implemented)\n");
	if (floor == 6)
		sprintf(temp, "[*] Floor 6: (not yet implemented)\n");
	else
		sprintf(temp, "[ ] Floor 6: (not yet implemented)\n");
	if (floor == 5)
		sprintf(temp, "[*] Floor 5: (not yet implemented)\n");
	else
		sprintf(temp, "[ ] Floor 5: (not yet implemented)\n");
	if (floor == 4)
		sprintf(temp, "[*] Floor 4: (not yet implemented)\n");
	else
		sprintf(temp, "[ ] Floor 4: (not yet implemented)\n");
	if (floor == 3)
		sprintf(temp, "[*] Floor 3: (not yet implemented)\n");
	else 
		sprintf(temp, "[ ] Floor 3: (not yet implemented)\n");
	if (floor == 2)
		sprintf(temp, "[*] Floor 2: (not yet implemented)\n");
	else
		sprintf(temp, "[ ] Floor 2: (not yet implemented)\n");
	if (floor == 1)
		sprintf(temp, "[*] Floor 1: (not yet implemented)\n");
	else
		sprintf(temp, "[ ] Floor 1: (not yet implemented)\n");
	len = strlen(temp);
	size = (sizeof(char)*len);
	strcpy(msg,temp);
	kfree(temp);
	return simple_read_from_buffer(ubuf, count, ppos, msg, size);	
}

// procfile write (m)
static ssize_t procfile_write(struct file* file, const char* ubuf, size_t count, loff_t* ppos){
	printk(KERN_INFO "proc_write\n");
	if(count > BUF_LEN)
		procfs_buf_len = BUF_LEN;
	else 
		procfs_buf_len = count;
	copy_from_user(msg, ubuf, procfs_buf_len);
	printk(KERN_INFO "got from user: %s\n", msg);

	return procfs_buf_len;
}

// file pointer (m)
static struct file_operations procfile_fops = {
	.owner = THIS_MODULE,
	.read = procfile_read,
	.write = procfile_write,
};


// intitialization function (m)
int systemCalls_init(void) {

	STUB_start_elevator = start_elevator;
	STUB_issue_request = issue_request;
	STUB_stop_elevator = stop_elevator;

	proc_entry = proc_create("elevator", 0666, NULL, &procfile_fops);	// setting proc entry point

	INIT_LIST_HEAD(&floor1);
	INIT_LIST_HEAD(&floor2);
	INIT_LIST_HEAD(&floor3);
	INIT_LIST_HEAD(&floor4);
	INIT_LIST_HEAD(&floor5);
	INIT_LIST_HEAD(&floor6);
	INIT_LIST_HEAD(&floor7);
	INIT_LIST_HEAD(&floor8);
	INIT_LIST_HEAD(&floor9);
	INIT_LIST_HEAD(&floor10);
	INIT_LIST_HEAD(&elevator);
	return 0;
}
module_init(systemCalls_init);

// exit function (m)
static void systemCalls_exit(void) {
	struct list_head* temp;
	struct list_head* dummy;
	Worker* worker;

	STUB_start_elevator = NULL;
	STUB_issue_request = NULL;
	STUB_stop_elevator = NULL;
	proc_remove(proc_entry);

	kfree(msg);
	list_for_each_safe(temp, dummy, &floor1){
		worker = list_entry(temp, Worker, list);
		list_del(temp);
		kfree(worker);
	}
	list_for_each_safe(temp, dummy, &floor2){
		worker = list_entry(temp, Worker, list);
		list_del(temp);
		kfree(worker);
	}
	list_for_each_safe(temp, dummy, &floor3){
		worker = list_entry(temp, Worker, list);
		list_del(temp);
		kfree(worker);
	}
	list_for_each_safe(temp, dummy, &floor4){
		worker = list_entry(temp, Worker, list);
		list_del(temp);
		kfree(worker);
	}
	list_for_each_safe(temp, dummy, &floor5){
		worker = list_entry(temp, Worker, list);
		list_del(temp);
		kfree(worker);
	}
	list_for_each_safe(temp, dummy, &floor6){
		worker = list_entry(temp, Worker, list);
		list_del(temp);
		kfree(worker);
	}
	list_for_each_safe(temp, dummy, &floor7){
		worker = list_entry(temp, Worker, list);
		list_del(temp);
		kfree(worker);
	}
	list_for_each_safe(temp, dummy, &floor8){
		worker = list_entry(temp, Worker, list);
		list_del(temp);
		kfree(worker);
	}
	list_for_each_safe(temp, dummy, &floor9){
		worker = list_entry(temp, Worker, list);
		list_del(temp);
		kfree(worker);
	}
	list_for_each_safe(temp, dummy, &floor10){
		worker = list_entry(temp, Worker, list);
		list_del(temp);
		kfree(worker);
	}

}
module_exit(systemCalls_exit);


