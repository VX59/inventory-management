#define FMT_HEADER_ONLY
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <sqlite3.h>
#include <string>
#include <fmt/core.h>
#include <thread>
#include <chrono>
#include <mutex>

class SQLDatabase {
protected:
	const char* wd = "C:\Users\deros\Documents\Database";
	static int callback(void* NotUsed, int argc, char** argv, char** asColName) {
		for (int i = 0; i < argc; i++) {
			std::cout << fmt::format("|{}: {:<{}}", asColName[i], argv[i], 12);
		}
		std::cout << std::endl;
		return 0;
	}
	static void validatePrepareStmt(int ret, sqlite3 *DBhandle) {
		if (ret != SQLITE_OK) {
			sqlite3_close(DBhandle);
			std::cerr << "failed to prepare statment";
			exit(- 1);
		}
	}
	static void validatestep(int ret, sqlite3* DBhandle) {
		if (ret != SQLITE_DONE) {
			sqlite3_close(DBhandle);
			std::cerr << "failed to execute query";
			exit(-1);
		}
	}
	int queryDB(std::string query, static int (*cb)(void* NotUsed, int argc, char** argv, char** asColName), char* messageError) {
		sqlite3* DBhandle;
		int exit = sqlite3_open(wd, &DBhandle);
		exit = sqlite3_exec(DBhandle, query.c_str(), cb, 0, &messageError);
		return exit;
	}

	int createDB() {
		sqlite3* DBhandle;
		sqlite3_open(wd, &DBhandle);
		sqlite3_close(DBhandle);
		return 0;
	}
	int showAllRecords(std::string table) {
		std::string query = "SELECT * FROM "+table+";";
		int exit = queryDB(query, callback, NULL);
		return 0;
	}
	int removeRecord(int id, std::string table) {
		char* messageError;
		sqlite3* DBhandle;
		sqlite3_open(wd, &DBhandle);
		sqlite3_stmt* sqlStmt = nullptr;
		std::string query = "DELETE FROM "+table+" WHERE ID = ?1";
		int exit = sqlite3_prepare_v2(DBhandle, query.c_str(), query.size() + 1, &sqlStmt, nullptr);
		validatePrepareStmt(exit, DBhandle);
		sqlite3_bind_int(sqlStmt, 1, id);
		exit = sqlite3_step(sqlStmt);
		validatestep(exit, DBhandle);
		sqlite3_finalize(sqlStmt);
		return 0;
	}
	int dropTable(std::string table) {
		char* messageError = (char*)malloc(1024);
		std::string query = "DROP TABLE "+table+";";
		int exit = queryDB(query, NULL, messageError);
		return 0;
	}
};

class SQLTransactions : SQLDatabase {

	int createTable() {
		sqlite3* DBhandle;
		std::string query = "CREATE TABLE transactions ("
			"ID integer PRIMARY KEY,"
			"STORENAME TEXT,"
			"s"
			"UNITS INT,"
			"PRICE FLOAT,"
			"CAPACITY INT,"
			"CATEGORY TEXT)";

		try {
			char* messageError = (char*)malloc(64);
			int exit = queryDB(query, callback, messageError);

			if (exit != SQLITE_OK) {
				std::cerr << "Error creating table" << std::endl;
				sqlite3_free(messageError);
			}
		}
		catch (const std::exception& e) {
			std::cerr << e.what();
		}
		return 0;
	}
};

class SQLInventory : SQLDatabase {
	typedef struct newItem {
		const char* NAME;
		const char* CATEGORY;
		float PRICE;
		const int ID;
		float CAPACITY;
		float UNITS;
	};
	int createTable() {
		sqlite3* DBhandle;
		std::string query = "CREATE TABLE inventory ("
			"ID integer PRIMARY KEY,"
			"NAME TEXT,"
			"UNITS INT,"
			"PRICE FLOAT,"
			"CAPACITY INT,"
			"CATEGORY TEXT)";

		try {
			char* messageError = (char*)malloc(64);
			int exit = queryDB(query, callback, messageError);

			if (exit != SQLITE_OK) {
				std::cerr << "Error creating table" << std::endl;
				sqlite3_free(messageError);
			}
		}
		catch (const std::exception& e) {
			std::cerr << e.what();
		}
		return 0;
	}
	int insertRecord(newItem* record) {
		char* messageError;
		sqlite3* DBhandle;
		int exit = sqlite3_open(wd, &DBhandle);
		sqlite3_stmt* sqlStmt = nullptr;
		std::string query = "INSERT INTO items (ID, NAME, UNITS, PRICE, CAPACITY, CATEGORY) VALUES( ?1, ?2, ?3, ?4, ?5, ?6);";
		exit = sqlite3_prepare_v2(DBhandle, query.c_str(), query.size() + 1, &sqlStmt, nullptr);
		validatePrepareStmt(exit, DBhandle);

		sqlite3_bind_int(sqlStmt, 1, record->ID);
		sqlite3_bind_text(sqlStmt, 2, record->NAME, -1, NULL);
		sqlite3_bind_int(sqlStmt, 3, record->UNITS);
		sqlite3_bind_int(sqlStmt, 4, record->PRICE);
		sqlite3_bind_int(sqlStmt, 5, record->CAPACITY);
		sqlite3_bind_text(sqlStmt, 6, record->CATEGORY, -1, NULL);

		exit = sqlite3_step(sqlStmt);
		validatestep(exit, DBhandle);
		sqlite3_finalize(sqlStmt);
		return 0;
	}
	int updateUnit(int newval, const char* condval, bool incr = false) {
		char* messageError;
		sqlite3* DBhandle;
		int exit = sqlite3_open(wd, &DBhandle);
		sqlite3_stmt* sqlStmt = nullptr;
		std::string query = "UPDATE items SET UNITS = ?1 WHERE NAME = ?2;";
		std::string incr_query = "UPDATE items SET UNITS = UNITS + ?1 WHERE NAME = ?2";
		if (incr == true) exit = sqlite3_prepare_v2(DBhandle, incr_query.c_str(), incr_query.size() + 1, &sqlStmt, nullptr);
		else exit = sqlite3_prepare_v2(DBhandle, query.c_str(), query.size() + 1, &sqlStmt, nullptr);
		validatePrepareStmt(exit, DBhandle);

		sqlite3_bind_int(sqlStmt, 1, newval);
		sqlite3_bind_text(sqlStmt, 2, condval, -1, NULL);

		exit = sqlite3_step(sqlStmt);
		validatestep(exit, DBhandle);
		sqlite3_finalize(sqlStmt);

		return 0;
	}
	int updatePrice(float newval, const char* condval, bool incr = false) {
		char* messageError;
		sqlite3* DBhandle;
		int exit = sqlite3_open(wd, &DBhandle);
		sqlite3_stmt* sqlStmt = nullptr;
		std::string query = "UPDATE items SET PRICE = ?1 WHERE NAME = ?2;";
		std::string incr_query = "UPDATE items SET PRICE = UNITS + ?1 WHERE NAME = ?2";
		if (incr == true) exit = sqlite3_prepare_v2(DBhandle, incr_query.c_str(), incr_query.size() + 1, &sqlStmt, nullptr);
		else exit = sqlite3_prepare_v2(DBhandle, query.c_str(), query.size() + 1, &sqlStmt, nullptr);
		validatePrepareStmt(exit, DBhandle);

		sqlite3_bind_double(sqlStmt, 1, newval);
		sqlite3_bind_text(sqlStmt, 2, condval, -1, NULL);

		exit = sqlite3_step(sqlStmt);
		validatestep(exit, DBhandle);
		sqlite3_finalize(sqlStmt);

		return 0;
	}
	int updateCapacity(int newval, const char* condval, bool incr = false) {
		char* messageError;
		sqlite3* DBhandle;
		int exit = sqlite3_open(wd, &DBhandle);
		sqlite3_stmt* sqlStmt = nullptr;
		std::string query = "UPDATE items SET CAPACITY = ?1 WHERE NAME = ?2;";
		std::string incr_query = "UPDATE items SET CAPACITY = UNITS + ?1 WHERE NAME = ?2";
		if (incr == true) exit = sqlite3_prepare_v2(DBhandle, incr_query.c_str(), incr_query.size() + 1, &sqlStmt, nullptr);
		else exit = sqlite3_prepare_v2(DBhandle, query.c_str(), query.size() + 1, &sqlStmt, nullptr);
		validatePrepareStmt(exit, DBhandle);

		sqlite3_bind_int(sqlStmt, 1, newval);
		sqlite3_bind_text(sqlStmt, 2, condval, -1, NULL);

		exit = sqlite3_step(sqlStmt);
		validatestep(exit, DBhandle);
		sqlite3_finalize(sqlStmt);

		return 0;
	}
};

class Warehouse : SQLInventory {
	int total_capacity;
	SQLInventory Inventory;
	SQLTransactions TCluster;
	int ClusterSize;
	Store** LocalCluster;

protected:
	struct restock_request {
		struct component {
			int item_id;
			int units;
		};
		int max_components = 10;
		std::string store;
		float weight;
		struct component** items;
	};
	class PQueue {
		typedef struct Node {
			struct restock_request* request;
			float weight;
			Node* next = nullptr;
		};
		Node* head = nullptr;
		int slots = 10;

	public:
		std::mutex mtx;
		void enqueue(struct restock_request* request) {
			Node *NewNode = (Node*)malloc(sizeof(Node));
			NewNode->request = request;
			NewNode->weight = request->weight;
			if (head == nullptr) {
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
		}
		Node* dequeue() {
			if (head != nullptr) {
				Node* alpha = head;
				head = head->next;
				return alpha;
			}
		}
	};
public:
	PQueue PriorityQueue;
};

class Store : SQLInventory, Warehouse {
	float* revenues;
	int total_capacity = 1000;
	float weight = 1.0;
	float* location;
	Warehouse supplier;
	SQLInventory Inventory;
	enum warehouse_request_type {UPDATE, SELECT};
	
	float compute_weight(); // softmax algorithm

public:
	// optional user interface, automatic triggered once a day
	restock_request* create_restock_request() {
		// access the store inventory
		// look for items that are low or out of stock
		// create a restock request object and add items to it
		// compute the weight of the request
		// submit the request
	}
	int submit_restock_request(restock_request* request) { // submits restock_request object to a priority queue
		supplier.PriorityQueue.enqueue(request);
	}
	int local_transaction(int itemID, int units) {
		// create a record in the local cluster's transaction table
	}


};
std::mutex mtx;

void consumer(void) {
	for (int i = 0; i < 10; i++) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		mtx.lock();
		Inventory.updateUnit(-1, "gold", true);
		Inventory.showAllRecords();
		mtx.unlock();
		std::cout << "Thread b Reporting: " << i << std::endl;
	}
}
void producer(void) {
	for (int i = 0; i < 10; i++) {
		//std::this_thread::sleep_for(std::chrono::milliseconds(20));
		mtx.lock();
		Inventory.updateUnit(2, "gold", true);
		Inventory.showAllRecords();

		mtx.unlock();
		std::cout << "Thread a Reporting: " << i << std::endl;
	}
}
int main() {
	std::thread pthreada(consumer);
	std::thread pthreadb(producer);
	//Item item = { "iron", "metal", 15.00, 1, 500, 10 };
	//Inventory.createTable();
	//Inventory.insertRecord(&item);
	//Inventory.showAllRecords();
	//Inventory.updatePrice(25, "iron");
	//Inventory.updateCapacity(1500, "iron");
	//Inventory.removeRecord("iron");
	pthreada.join();
	pthreadb.join();
	//Inventory.dropTable();
	return 0;
}