#pragma once

#define FMT_HEADER_ONLY
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <windows.h>
#include <cstdlib>
#include <sqlite3.h>
#include <string>
#include <fmt/core.h>
#include <thread>
#include <chrono>
#include <mutex>

typedef struct Tcomponent {
	int itemunits;
	float itemprice;
	int itemId;
	const char* itemName;
	Tcomponent* next;
};

typedef struct newTransaction {
	Tcomponent* itemListHead;
	int transactionid;
	int storeid;
	char* store;
	float computeCount() {
		float total = 0;
		Tcomponent* alpha = itemListHead;
		while (alpha->next != nullptr) {
			total += alpha->itemunits;
			alpha = alpha->next;
		}
		return total;
	}
	float computeVolume() {
		float total = 0;
		Tcomponent* alpha = itemListHead;
		while (alpha->next != nullptr) {
			total += alpha->itemprice;
			alpha = alpha->next;
		}
		return total;
	}
	int count = computeCount();
	float volume = computeVolume();
};

typedef struct newItem {
	const char* NAME;
	const char* CATEGORY;
	float PRICE;
	const int ID;
	float CAPACITY;
	float UNITS;
};

class component {
public:
	int item_id;
	int units;
	component* next;
};

class restock_request {
public:
	int components = 0;
	component* itemListHead;
	int CreateComponent(int item_id, int units) {
		component* cmpnt = new component();
		cmpnt->item_id = item_id;
		cmpnt->units = units;
		if (components == 0) {
			itemListHead = cmpnt;
			itemListHead->next = nullptr;
		}
		else {
			cmpnt->next = itemListHead;
			itemListHead = cmpnt;
		}
		components++;
		return 0;
	}
	float weight;
	int store_id;
	char* table_name;
	int max_components = 10;
};

typedef struct Node {
public:
	restock_request* request;
	float weight;
	Node* next = nullptr;
};

typedef struct PQueue {
	int slots = 10;
	int occupied_slots = 0;
public:
	Node* head;

	int check_duplicates(restock_request* request) {
		if (occupied_slots == 0) {
			return 0;
		}
		Node* alpha = head;
		while (alpha != nullptr) {
			if (alpha->request->store_id == request->store_id) {
				return -1;
			}
			alpha = alpha->next;
		}
		return 0;
	}

	int enqueue(restock_request* request) {

		if (check_duplicates(request) != 0) return -1;

		Node* NewNode = new Node();
		NewNode->request = request;
		NewNode->weight = request->weight;

		if (occupied_slots == 0) {
			head = NewNode;
		}
		else {
			if (NewNode->weight > head->weight) {
				NewNode->next = head;
				head = NewNode;
			}
			else {
				Node* alpha = head;
				while (NewNode->weight < alpha->weight) {
					if (alpha->next != nullptr && NewNode->weight > alpha->next->weight) {
						NewNode->next = alpha->next;
						alpha->next = NewNode;
					}
					if (alpha->next == nullptr) {
						alpha->next = NewNode;
					}
					alpha = alpha->next;
				}
			}
		}
		occupied_slots++;
		return 0;
	}
	restock_request* dequeue() {
		if (occupied_slots > 0) {
			Node* alpha = head;
			restock_request* request = alpha->request;
			head = head->next;
			occupied_slots--;
			return request;
		}
	}
	void peek() {
		if (occupied_slots > 0) {
			component* alpha = head->request->itemListHead;
			while (alpha != nullptr) {
				std::cout << "weight | " << head->request->weight << " item id | " << alpha->item_id << " units | " << alpha->units << std::endl;
				alpha = alpha->next;
			}
		}
	}
};