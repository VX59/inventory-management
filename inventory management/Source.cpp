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
public:
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
			std::cerr << "failed to prepare statment" << ret << std::endl;
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

typedef struct Tcomponent {
	int itemunits;
	float itemprice;
	int itemId;
	const char* itemName;
	Tcomponent* next;
};

typedef struct newTransaction {
	Tcomponent*itemListHead;
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
		while(alpha->next != nullptr) {
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

class SQLInventory : SQLDatabase {
public:
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
		std::string query = "INSERT INTO inventory (ID, NAME, UNITS, PRICE, CAPACITY, CATEGORY) VALUES( ?1, ?2, ?3, ?4, ?5, ?6);";
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
		std::string query = "UPDATE inventory SET UNITS = ?1 WHERE NAME = ?2;";
		std::string incr_query = "UPDATE inventory SET UNITS = UNITS + ?1 WHERE NAME = ?2";
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
		std::string query = "UPDATE inventory SET PRICE = ?1 WHERE NAME = ?2;";
		std::string incr_query = "UPDATE inventory SET PRICE = UNITS + ?1 WHERE NAME = ?2";
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
		std::string query = "UPDATE inventory SET CAPACITY = ?1 WHERE NAME = ?2;";
		std::string incr_query = "UPDATE inventory SET CAPACITY = UNITS + ?1 WHERE NAME = ?2";
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
	void showAll() {
		showAllRecords("inventory");
	}
};

class SQLTransactions : SQLDatabase {
public:
	int createTable() {
		sqlite3* DBhandle;
		std::string query = "CREATE TABLE transactions ("
			"ID INT PRIMARY KEY,"
			"STORENAME TEXT,"
			"STOREID INT"
			"UNITS INT,"
			"PRICE FLOAT)";

		try {
			char* messageError = (char*)malloc(64);
			int exit = queryDB(query, callback, messageError);

			if (exit != SQLITE_OK) {
				std::cerr << "Error creating table " << exit << std::endl;
				sqlite3_free(messageError);
			}
		}
		catch (const std::exception& e) {
			std::cerr << e.what();
		}
		return 0;
	}

	int insertRecord(newTransaction *record) {
		char messageError;
		sqlite3* DBhandle;
		int exit = sqlite3_open(wd, &DBhandle);
		Tcomponent *alpha = record->itemListHead;
		while (alpha->next != nullptr) {
			std::string itemName = alpha->itemName;
			std::string query = "INSERT INTO transactions (ID, STORENAME, STOREID, UNITS, PRICE, " + itemName + ") VALUES(?1, ?2, ?3, ?4, ?5, ?6)";
			std::cout << query << std::endl;
			sqlite3_stmt* sqlStmt = nullptr;
			exit = sqlite3_prepare_v2(DBhandle, query.c_str(), query.size() + 1, &sqlStmt, nullptr);
			validatePrepareStmt(exit, DBhandle);

			sqlite3_bind_int(sqlStmt, 1, record->transactionid);
			sqlite3_bind_text(sqlStmt, 2, record->store, -1, NULL);
			sqlite3_bind_int(sqlStmt, 3, record->storeid);
			sqlite3_bind_int(sqlStmt, 4, alpha->itemunits);
			sqlite3_bind_int(sqlStmt, 5, alpha->itemprice);
			sqlite3_bind_text(sqlStmt, 6, alpha->itemName, -1, NULL);

			exit = sqlite3_step(sqlStmt);
			validatestep(exit, DBhandle);
			sqlite3_finalize(sqlStmt);

			alpha = alpha->next;
		}
		return 0;
	}

	int updateTransactionCols() {
		char* messageError = (char*)malloc(64);
		sqlite3* DBhandle;
		sqlite3_open(wd, &DBhandle);
		sqlite3_stmt* sqlStmt = nullptr;
		std::string query = "SELECT NAME FROM inventory";
		int exit = sqlite3_prepare_v2(DBhandle, query.c_str(), query.size() + 1, &sqlStmt, nullptr);
		validatePrepareStmt(exit, DBhandle);
		while (exit = sqlite3_step(sqlStmt) != SQLITE_DONE) {
			std::string name = (const char*)sqlite3_column_text(sqlStmt, 0);
			std::string query = "ALTER transactions ADD" + name + " TEXT;";
			exit = sqlite3_exec(DBhandle, query.c_str(), NULL, 0, &messageError);
		}
		sqlite3_finalize(sqlStmt);
		return 0;
	}
	void showAll() {
		showAllRecords("transactions");
	}
};

class Warehouse : SQLInventory {
	int total_capacity;
	SQLInventory Inventory;
	SQLTransactions TCluster;
	int ClusterSize;

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
	PQueue Requests;
};

class Store : SQLInventory, SQLTransactions, Warehouse {
	float* revenues;
	int total_capacity = 1000;
	float weight = 1.0;
	float* location;
	Warehouse supplier;
	SQLInventory Inventory;
	SQLTransactions StoreTransactions;
	enum warehouse_request_type {UPDATE, SELECT};
	
	float compute_weight(); // softmax algorithm?

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
		supplier.Requests.enqueue(request);
		return 0;
	}
	int local_transaction(newTransaction *record) {
		//updateTransactionCols();
		StoreTransactions.insertRecord(record);
		return 0;
	}
};

int main() {
	newItem itema = { "iron", "metal", 15.00, 1, 5000, 1310 };
	newItem itemb = { "platinum", "metal", 15.00, 2, 500, 140 };
	newItem itemc = { "aluminum", "metal", 15.00, 3, 500, 10 };
	
	Tcomponent Tcmpc = { 6, 15.00, 3, "aluminum", nullptr };
	Tcomponent Tcmpb = { 15, 15.00, 2, "platinum", &Tcmpc };
	Tcomponent Tcmpa = { 5, 15.00, 1, "iron", &Tcmpb };

	newTransaction Transaction = { &Tcmpa,0, 0, (char*)"slave auction" };

	SQLDatabase database;
	//database.dropTable("items");
	SQLInventory inventory;
	Store store;
	SQLTransactions TCluster;
	//inventory.createTable();
	//inventory.insertRecord(&itema);
	//inventory.insertRecord(&itemb);
	//inventory.insertRecord(&itemc);
	//inventory.showAll();
	//TCluster.createTable();
	store.local_transaction(&Transaction);
	TCluster.showAll();
	return 0;
}