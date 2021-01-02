#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp() {
	WP *new, *p;
	new = free_;
	free_ = free_->next;
	new->next = NULL;
	p = head;
	if (p == NULL)
	{
		head = new;
		p = head;
	}
	else 
	{
		while (p->next != NULL)
		{
			p = p->next;
		}
		p->next = new;
		
	}
	return new;
}

void free_wp(WP *wp) {
	WP *f, *p;
	p = free_;
	if (p == NULL)
	{
		free_ = wp;
		p = free_;
	}
	else 
	{
		while (p->next != NULL)
		{
			p = p->next;
		}
		p->next = wp;
		printf("watch-point %d is released.\n", wp->NO);
		
	}

	f = head;
	if (head == NULL)
	{
		printf("watch point pool is empty!\n");
		assert(0);
	} 
	
	if (head->NO == wp->NO)
	{
		head = head->next;
	}
	else
	{
		while (f->next != NULL && f->NO != wp->NO)
		{
			f = f->next;
		}
		if (f->next == NULL && f->NO == wp->NO)
		{
			printf("something goes wrong!");
		}
		else if (f->next->NO == wp->NO)
		{
			f->next = f->next->next;
			
		}
		else assert(0);
	}

	wp->next = NULL;
	wp->value = 0;
	
}

bool check_wp() {
	WP *f;
	f = head;
	bool key = false;
	bool suc;
	while (f != NULL)
	{
		uint32_t tmp_expr = expr(f->expr, &suc);
		if (suc == false) 
		{
			printf("evaluation failed!\n");
			assert(0);
		}

		if (tmp_expr != f->value)
		{
			
			key = true;
			printf("Watch-point %d: %s\n", f->NO, f->expr);
			printf("Origin value: %d\n", f->value);
			printf("New value: %d\n", tmp_expr);
		}
		f = f->next;
		
		
	}
	return key;
	
}

void delete_wp(int num) {
	WP *f;
	f = &wp_pool[num];
	free_wp(f);
}

void info_wp() {
	WP *f;
	f = head;
	if (f == NULL)
	{
		printf("There is no watch-point.");
	}
	
	while (f != NULL)
	{
		printf("Watch-point %d: %s = %d\n", f->NO, f->expr, f->value);
		f = f->next;
	}
	
}
