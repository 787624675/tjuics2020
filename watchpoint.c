#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "nemu.h"
#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;


void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	//	wp_pool[i].is_free = true;
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp() {
	if(free_ == NULL){
		assert(0);
	}else{
		WP* p = free_;
		WP* q = free_->next;
		p ->next = NULL;
		free_ = q;
		if(head == NULL){
			head = p;
		}else{
			WP* p1 = head;
			while(p1->next != NULL){
				p1 = p1->next;
			}
			p1->next = p;
		}
		return p;

	}
}
void free_wp(uint32_t num) {
	WP *p = head;
	WP *q = head;
	WP *r = free_; 
	
	if(p == NULL){
		printf("No watchpoints to free");
	}else if(head->next == NULL){
		head = NULL;
	}
	else{
		do{
			if(p->NO == num){
				q->next = p->next;
				p->next = NULL;
				break;
			}
			p = p->next;
			if(p->NO != num){
				q = q->next;
			}
		
		}while(p != NULL);
	}
	if(r == NULL){
		r = p;
	}
	else{
		while(r->next != NULL){
			r = r->next;
		}
		r->next = p;
	}
}
WP*  get_wp(){
	return head;
}
void print_wp(int num){
	WP* wp = head;
	if(wp == NULL){
		printf("There is no watchpoints");
	}else{
		int i = 0;
		do{
			i++;
			if(i == num){
				printf("NO.%d\texpression:%s\tvalue:%d\n",wp->NO,wp->expr,wp->value);
				}
			wp = wp->next;
		}while(wp!=NULL);
	}

}
void ui_mainloop();
uint32_t expr(char *e, bool *success);
uint32_t check_wp(){
	WP* p = head;
	bool flag = true;
	bool success;
	int i = 1;
	while(p!=NULL){
		uint32_t res = expr(p->expr, &success);
		if(res!=p->value){
			flag = false;
			printf("Hit watch point NO.%d at 0x %08x, new value: %d\nold info:\t",p->NO, cpu.eip, res);
			print_wp(i);
			
		}
		p = p->next;
		i++;
	}
	if(flag == false){
		return 1;
	}
	else{
		return 0;
	}
}
