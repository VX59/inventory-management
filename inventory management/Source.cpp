#define FMT_HEADER_ONLY
#define _CRT_SECURE_NO_WARNINGS
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
		for (int i = 0; i < argc; i++) 
		{
			std::string value;
			if (argv[i] == nullptr)
					value = "";
			else
				value = argv[i];

				std::cout << fmt::format("|{}: {:<{}}", asColName[i], value, 12);
		}
		std::cout << std::endl;
		return 0;
	}
	static void validatePrepareStmt(int ret, sqlite3* DBhandle) {
		if (ret != SQLITE_OK) {
			sqlite3_close(DBhandle);
			std::cerr << "failed to prepare statment" << ret << std::endl;
			exit(-1);
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
		sqlite3_close(DBhandle);
		return exit;
	}

	int createDB() {
		sqlite3* DBhandle;
		sqlite3_open(wd, &DBhandle);
		sqlite3_close(DBhandle);
		return 0;
	}
	int showAllRecords(std::string table) {
		std::string query = "SELECT * FROM " + table + ";";
		int exit = queryDB(query, callback, NULL);
		return 0;
	}
	int removeRecord(int id, std::string table) {
		char* messageError;
		sqlite3* DBhandle;
		sqlite3_open(wd, &DBhandle);
		sqlite3_stmt* sqlStmt = nullptr;
		std::string query = "DELETE FROM " + table + " WHERE ID = ?1";
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
		std::string query = "DROP TABLE " + table + ";";
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
typedef struct restock_request {
	typedef struct component {
		const char* item_name;
		int units;
		component* next;
	};
	struct component* itemListHead = nullptr;

	int CreateComponent(const char* item_name, int units) {
		component *cmpnt = (component*)malloc(sizeof(component));
		cmpnt->item_name = item_name;
		cmpnt->units = units;
		if (itemListHead == nullptr) itemListHead = cmpnt;
		else {
			cmpnt->next = itemListHead;
			itemListHead = cmpnt;
		}
		return 0;
	}
	int max_components = 10;
	std::string store;
	float weight;
};

class SQLInventory : SQLDatabase {
public:
	int createTable(std::string name) {
		sqlite3* DBhandle;
		std::string query = "CREATE TABLE "+name+" ("
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
	int insertRecord(std::string table, newItem* record) {
		char* messageError;
		sqlite3* DBhandle;
		int exit = sqlite3_open(wd, &DBhandle);
		sqlite3_stmt* sqlStmt = nullptr;
		std::string query = "INSERT INTO "+table+" (ID, NAME, UNITS, PRICE, CAPACITY, CATEGORY) VALUES( ?1, ?2, ?3, ?4, ?5, ?6);";
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
		sqlite3_close(DBhandle);
		return 0;
	}
	int updateUnit(std::string table, int newval, const char* condval, bool incr = false) {
		char* messageError;
		sqlite3* DBhandle;
		int exit = sqlite3_open(wd, &DBhandle);
		sqlite3_stmt* sqlStmt = nullptr;
		std::string query = "UPDATE "+table+" SET UNITS = ?1 WHERE NAME = ?2;";
		std::string incr_query = "UPDATE "+table+" SET UNITS = UNITS + ?1 WHERE NAME = ?2";
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
	int updatePrice(std::string table, float newval, const char* condval, bool incr = false) {
		char* messageError;
		sqlite3* DBhandle;
		int exit = sqlite3_open(wd, &DBhandle);
		sqlite3_stmt* sqlStmt = nullptr;
		std::string query = "UPDATE " + table + " SET PRICE = ?1 WHERE NAME = ?2;";
		std::string incr_query = "UPDATE " + table + " SET PRICE = UNITS + ?1 WHERE NAME = ?2";
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
	int updateCapacity(std::string table, int newval, const char* condval, bool incr = false) {
		char* messageError;
		sqlite3* DBhandle;
		int exit = sqlite3_open(wd, &DBhandle);
		sqlite3_stmt* sqlStmt = nullptr;
		std::string query = "UPDATE " + table + " SET CAPACITY = ?1 WHERE NAME = ?2;";
		std::string incr_query = "UPDATE " + table + " SET CAPACITY = UNITS + ?1 WHERE NAME = ?2";
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
	int createTable(std::string name) {
		sqlite3* DBhandle;
		std::string query = "CREATE TABLE "+name+" ("
			"TCOUNT INTEGER PRIMARY KEY AUTOINCREMENT,"
			"TID INT,"
			"STORENAME TEXT,"
			"STOREID INT,"
			"PRICE FLOAT,"
			"UNITS INT)";

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

	int insertRecord(const char* storeName, int storeID, int tID, Tcomponent* record) {
		char* messageError;
		sqlite3* DBhandle;
		int exit = sqlite3_open(wd, &DBhandle);
		sqlite3_stmt* sqlStmt = nullptr;
		std::string query = "INSERT INTO transactions (TID, STORENAME, STOREID, PRICE, UNITS, "+ (std::string)record->itemName +") VALUES(?1, ?2, ?3, ?4, ?5, ?6);";
		exit = sqlite3_prepare_v2(DBhandle, query.c_str(), query.size() + 1, &sqlStmt, nullptr);
		validatePrepareStmt(exit, DBhandle);

		sqlite3_bind_int(sqlStmt, 1, tID);
		sqlite3_bind_text(sqlStmt, 2, storeName, -1, NULL);
		sqlite3_bind_int(sqlStmt, 3, storeID);
		sqlite3_bind_double(sqlStmt, 4, record->itemprice);
		sqlite3_bind_int(sqlStmt, 5, record->itemunits);
		sqlite3_bind_int(sqlStmt, 6, 1);

		exit = sqlite3_step(sqlStmt);
		validatestep(exit, DBhandle);
		sqlite3_finalize(sqlStmt);
		sqlite3_close(DBhandle);
		return 0;
	}

	int updateTransactionCols() {
		char* messageError = (char*)malloc(32);
		sqlite3* DBhandle;
		sqlite3_stmt* sqlStmt = nullptr;
		std::string query = "SELECT * FROM store_inventory";
		sqlite3_open(wd, &DBhandle);
		int exit = sqlite3_prepare_v2(DBhandle, query.c_str(), query.size() + 1, &sqlStmt, nullptr);
		validatePrepareStmt(exit, DBhandle);

		while (sqlite3_step(sqlStmt) == SQLITE_ROW) {
			std::string colName = (const char*)sqlite3_column_text(sqlStmt, 1);
			query = "ALTER TABLE transactions ADD " + colName + " INT; ";
			exit = sqlite3_exec(DBhandle, query.c_str(), callback, 0, &messageError);
			//std::cout << colName << std::endl;
		}
		sqlite3_finalize(sqlStmt);

		query = "pragma table_info('transactions');";
		exit = sqlite3_prepare_v2(DBhandle, query.c_str(), query.size() + 1, &sqlStmt, nullptr);
		validatePrepareStmt(exit, DBhandle);

		while (sqlite3_step(sqlStmt) != SQLITE_DONE) {
			std::string tableName = (const char*)sqlite3_column_text(sqlStmt, 1);
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
			Node* NewNode = (Node*)malloc(sizeof(Node));
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
		void peek() {
			std::cout << head->request->weight;
		}
	};
public:
	PQueue Requests;
};

class Store : SQLDatabase, SQLInventory, SQLTransactions, Warehouse {
	float* revenues;
	int total_capacity = 1000;
	float weight = 1.0;
	float* location;
	enum warehouse_request_type { UPDATE, SELECT };
	Warehouse supplier;
	float compute_restock_request_weight() {
		restock_request* request = (restock_request*)malloc(sizeof(restock_request));
		sqlite3* DBhandle;
		sqlite3_stmt* sqlStmt;
		float threshhold = 0.05;
		std::string query = "SELECT STORENAME,PRICE,UNITS FROM transactions;";

		sqlite3_open(SQLDatabase::wd, &DBhandle);
		int exit = sqlite3_prepare(DBhandle, query.c_str(), query.size() + 1, &sqlStmt, nullptr);
		SQLDatabase::validatePrepareStmt(exit, DBhandle);
		int store_total_units = 0;
		float store_total_volume = 0;
		int total_units = 0;
		float total_volume = 0;
		while (exit = sqlite3_step(sqlStmt) == SQLITE_ROW) {
			int units = sqlite3_column_int(sqlStmt, 2);
			float price = sqlite3_column_double(sqlStmt, 1);
			total_units += units;
			total_volume += price;
			const char* store_name = (const char*)sqlite3_column_text(sqlStmt, 0);
			if (name == store_name) {
				store_total_units += units;
				store_total_volume += price;
			}
		}
		sqlite3_finalize(sqlStmt);
		sqlite3_close(DBhandle);
		float weight = (store_total_units * store_total_volume) / (total_units * total_volume);
		return weight;
	}

public:
	std::string name;
	// optional user interface, automatic triggered once a day
	int submit_restock_request() {
		restock_request *request = (restock_request *)malloc(sizeof(restock_request));
		//char buffer[16];
		//request->store = name.copy(buffer, name.size() + 1);

		//std::cout << request->store << std::endl;
		sqlite3* DBhandle;
		sqlite3_stmt* sqlStmt;
		float threshhold = 0.05;
		std::string query = "SELECT NAME, UNITS, CAPACITY FROM store_inventory;";

		sqlite3_open(SQLDatabase::wd, &DBhandle);
		int exit = sqlite3_prepare(DBhandle, query.c_str(), query.size() + 1, &sqlStmt, nullptr);
		SQLDatabase::validatePrepareStmt(exit, DBhandle);

		while (exit = sqlite3_step(sqlStmt) == SQLITE_ROW) {
			const char *name = (const char*)sqlite3_column_text(sqlStmt, 0);
			float units = sqlite3_column_int(sqlStmt, 1);
			float capacity = sqlite3_column_int(sqlStmt, 2);
			float pStock = units / capacity;
			std::cout << pStock << std::endl;
			if (pStock < threshhold) {
				int requested_units = capacity - units;
				request->CreateComponent(name, requested_units);
			}
		}
		sqlite3_finalize(sqlStmt);
		sqlite3_close(DBhandle);
		request->weight = compute_restock_request_weight();
		supplier.Requests.enqueue(request);
		supplier.Requests.peek();

		return 0;
	}
	int local_transaction(newTransaction* record) {
		SQLTransactions::updateTransactionCols();
		Tcomponent* alpha = record->itemListHead;	
		while (alpha != nullptr) {
			// modify the store inventory
			SQLInventory::updateUnit("store_inventory", -(alpha->itemunits), alpha->itemName, true);
			SQLTransactions::insertRecord(record->store, record->storeid, record->transactionid, alpha);
			alpha = alpha->next;
		}
		return 0;
	}
};

int main() {
	newItem itema = { "iron", "metal", 15.00, 1, 250, 15 };
	newItem itemb = { "platinum", "metal", 15.00, 2, 250, 33 };
	newItem itemc = { "aluminum", "metal", 15.00, 3, 250, 10 };

	Tcomponent Tcmpc = { 6, 15.00, 3, "aluminum", nullptr };
	Tcomponent Tcmpb = { 15, 15.00, 2, "platinum", &Tcmpc };
	Tcomponent Tcmpa = { 5, 15.00, 1, "iron", &Tcmpb };

	newTransaction Transaction = { &Tcmpa, 0, 0, (char*)"tyrant" };

	SQLDatabase database;
	//database.dropTable("items");
	SQLInventory inventory;
	Store store;
	store.name = "tyrant";
	SQLTransactions TCluster;
	//inventory.createTable();
	//inventory.showAll();
	//database.dropTable("transactions");
	//database.dropTable("store_inventory");

	//TCluster.createTable("transactions");
	//inventory.createTable("store_inventory");

	//inventory.insertRecord("store_inventory", &itema);
	//inventory.insertRecord("store_inventory", &itemb);
	//inventory.insertRecord("store_inventory", &itemc);

	//database.showAllRecords("store_inventory");
	//store.local_transaction(&Transaction);
	database.showAllRecords("transactions");
	database.showAllRecords("store_inventory");
	store.submit_restock_request();

	return 0;
}