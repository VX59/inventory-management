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
	static void validatePrepareStmt(int ret, sqlite3* DBhandle, std::string query) {
		std::cout << query << std::endl;
		if (ret != SQLITE_OK) {
			sqlite3_close(DBhandle);
			std::cerr << "failed to prepare statment" << ret << std::endl;
			exit(-1);
		}
	}
	static void validatestep(int ret, sqlite3* DBhandle) {
		if (ret != SQLITE_DONE && ret != SQLITE_ROW) {
			sqlite3_close(DBhandle);
			std::cerr << "failed to execute query";
			exit(ret);
		}
	}
	int queryDB(std::string query, int (*cb)(void* NotUsed, int argc, char** argv, char** asColName)) {
		char* messageError;
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
		int exit = queryDB(query, callback);
		return 0;
	}
	int removeRecord(int id, std::string table) {
		char* messageError;
		sqlite3* DBhandle;
		sqlite3_open(wd, &DBhandle);
		sqlite3_stmt* sqlStmt = nullptr;
		std::string query = "DELETE FROM " + table + " WHERE ID = ?1";
		int exit = sqlite3_prepare_v2(DBhandle, query.c_str(), query.size() + 1, &sqlStmt, nullptr);
		validatePrepareStmt(exit, DBhandle, query);
		sqlite3_bind_int(sqlStmt, 1, id);
		exit = sqlite3_step(sqlStmt);
		validatestep(exit, DBhandle);
		sqlite3_finalize(sqlStmt);
		return 0;
	}
	int dropTable(std::string table) {
		std::string query = "DROP TABLE " + table + ";";
		int ret = queryDB(query, NULL);
		if (ret != SQLITE_OK) {
			std::cerr << "Error dropping table " + table << std::endl;
			exit(-1);
		}
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
typedef struct newFacility {
	int ID;
	const char* NAME;
	const char* CLASS;
	const char* LOCAL_INVENTORY;
	const char* SUPPLY_INVENTORY;
	const char* TRANSACTIONS;
	newFacility* next;
};
class SQLFacility_Index : SQLDatabase {
public:
	int createTable(std::string name) {
		sqlite3* DBhandle;
		std::string query = "CREATE TABLE " + name + " ("
			"ID integer PRIMARY KEY,"
			"NAME TEXT,"
			"CLASS TEXT,"
			"LOCAL_INVENTORY TEXT,"
			"SUPPLY_INVENTORY TEXT,"
			"TRANSACTIONS TEXT)";

		try {
			int ret = queryDB(query, callback);

			if (ret != SQLITE_OK) {
				std::cerr << "Error creating table" << std::endl;
				exit(ret);
			}
		}
		catch (const std::exception& e) {
			std::cerr << e.what();
		}
		return 0;
	}
	int insertRecord(std::string table, newFacility *record) {
		sqlite3* DBhandle;
		int exit = sqlite3_open(wd, &DBhandle);
		sqlite3_stmt* sqlStmt = nullptr;
		std::string query = "INSERT INTO " + table + " (ID, NAME, CLASS, LOCAL_INVENTORY, SUPPLY_INVENTORY, TRANSACTIONS) VALUES( ?1, ?2, ?3, ?4, ?5, ?6);";
		exit = sqlite3_prepare_v2(DBhandle, query.c_str(), query.size() + 1, &sqlStmt, nullptr);
		validatePrepareStmt(exit, DBhandle, query);

		sqlite3_bind_int(sqlStmt, 1, record->ID);
		sqlite3_bind_text(sqlStmt, 2, record->NAME, -1, NULL);
		sqlite3_bind_text(sqlStmt, 3, record->CLASS, -1, NULL);
		sqlite3_bind_text(sqlStmt, 4, record->LOCAL_INVENTORY, -1, NULL);
		sqlite3_bind_text(sqlStmt, 5, record->SUPPLY_INVENTORY, -1, NULL);
		sqlite3_bind_text(sqlStmt, 6, record->TRANSACTIONS, -1, NULL);

		exit = sqlite3_step(sqlStmt);
		validatestep(exit, DBhandle);

		sqlite3_finalize(sqlStmt);
		sqlite3_close(DBhandle);
		return 0;
	}
};

class SQLInventory : SQLDatabase {
public:
	int createTable(std::string name) {
		sqlite3* DBhandle;
		std::string query = "CREATE TABLE " + name + " ("
			"ID integer PRIMARY KEY,"
			"NAME TEXT,"
			"UNITS INT,"
			"PRICE FLOAT,"
			"CAPACITY INT,"
			"CATEGORY TEXT)";

		try {
			int exit = queryDB(query, callback);

			if (exit != SQLITE_OK) {
				std::cerr << "Error creating table" << std::endl;
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
		std::string query = "INSERT INTO " + table + " (ID, NAME, UNITS, PRICE, CAPACITY, CATEGORY) VALUES( ?1, ?2, ?3, ?4, ?5, ?6);";
		exit = sqlite3_prepare_v2(DBhandle, query.c_str(), query.size() + 1, &sqlStmt, nullptr);
		validatePrepareStmt(exit, DBhandle, query);

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
	int updateUnit(std::string table, int newval, int condval, bool incr = false) {
		char* messageError;
		sqlite3* DBhandle;
		int exit = sqlite3_open(wd, &DBhandle);
		sqlite3_stmt* sqlStmt = nullptr;
		std::string query = "SELECT * FROM " + table + " WHERE ID = ?1;";
		exit = sqlite3_prepare_v2(DBhandle, query.c_str(), query.size() + 1, &sqlStmt, nullptr);
		validatePrepareStmt(exit, DBhandle, query);
		sqlite3_bind_int(sqlStmt, 1, condval);
		exit = sqlite3_step(sqlStmt);
		validatestep(exit, DBhandle);
		int units = sqlite3_column_int(sqlStmt, 2);
		sqlite3_finalize(sqlStmt);

		if (units + newval >= 0) {
			query = "UPDATE " + table + " SET UNITS = ?1 WHERE ID = ?2;";
			std::string incr_query = "UPDATE " + table + " SET UNITS = UNITS + ?1 WHERE ID = ?2";
			if (incr == true) exit = sqlite3_prepare_v2(DBhandle, incr_query.c_str(), incr_query.size() + 1, &sqlStmt, nullptr);
			else exit = sqlite3_prepare_v2(DBhandle, query.c_str(), query.size() + 1, &sqlStmt, nullptr);
			validatePrepareStmt(exit, DBhandle, query);
			validatePrepareStmt(exit, DBhandle, query);

			sqlite3_bind_int(sqlStmt, 1, newval);
			sqlite3_bind_int(sqlStmt, 2, condval);

			exit = sqlite3_step(sqlStmt);
			validatestep(exit, DBhandle);
			sqlite3_finalize(sqlStmt);
			sqlite3_close(DBhandle);

			return 1;
		}
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
		validatePrepareStmt(exit, DBhandle, query);

		sqlite3_bind_double(sqlStmt, 1, newval);
		sqlite3_bind_text(sqlStmt, 2, condval, -1, NULL);

		exit = sqlite3_step(sqlStmt);
		validatestep(exit, DBhandle);
		sqlite3_finalize(sqlStmt);
		sqlite3_close(DBhandle);

		return 0;
	}
	int updateCapacity(std::string table, int newval, int condval, bool incr = false) {
		char* messageError;
		sqlite3* DBhandle;
		int exit = sqlite3_open(wd, &DBhandle);
		sqlite3_stmt* sqlStmt = nullptr;
		std::string query = "UPDATE " + table + " SET CAPACITY = ?1 WHERE ID = ?2;";
		std::string incr_query = "UPDATE " + table + " SET CAPACITY = UNITS + ?1 WHERE ID = ?2";
		if (incr == true) exit = sqlite3_prepare_v2(DBhandle, incr_query.c_str(), incr_query.size() + 1, &sqlStmt, nullptr);
		else exit = sqlite3_prepare_v2(DBhandle, query.c_str(), query.size() + 1, &sqlStmt, nullptr);
		validatePrepareStmt(exit, DBhandle, query);

		sqlite3_bind_int(sqlStmt, 1, newval);
		sqlite3_bind_int(sqlStmt, 2, condval);

		exit = sqlite3_step(sqlStmt);
		validatestep(exit, DBhandle);
		sqlite3_finalize(sqlStmt);
		sqlite3_close(DBhandle);

		return 0;
	}
};

class SQLTransactions : SQLDatabase {
public:
	int createTable(std::string name) {
		sqlite3* DBhandle;
		std::string query = "CREATE TABLE " + name + " ("
			"TCOUNT INTEGER PRIMARY KEY AUTOINCREMENT,"
			"TID INT,"
			"STORENAME TEXT,"
			"STOREID INT,"
			"PRICE FLOAT,"
			"UNITS INT)";

		try {
			int exit = queryDB(query, callback);

			if (exit != SQLITE_OK) {
				std::cerr << "Error creating table " << exit << std::endl;
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
		std::string query = "INSERT INTO transactions (TID, STORENAME, STOREID, PRICE, UNITS, " + (std::string)record->itemName + ") VALUES(?1, ?2, ?3, ?4, ?5, ?6);";
		exit = sqlite3_prepare_v2(DBhandle, query.c_str(), query.size() + 1, &sqlStmt, nullptr);
		validatePrepareStmt(exit, DBhandle, query);

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

	int updateTransactionCols(std::string table) {
		char* messageError;
		sqlite3* DBhandle;
		sqlite3_stmt* sqlStmt = nullptr;
		std::string query = "SELECT * FROM " + table;
		sqlite3_open(wd, &DBhandle);
		int exit = sqlite3_prepare_v2(DBhandle, query.c_str(), query.size() + 1, &sqlStmt, nullptr);
		validatePrepareStmt(exit, DBhandle, query);

		while (sqlite3_step(sqlStmt) == SQLITE_ROW) {
			std::string colName = (const char*)sqlite3_column_text(sqlStmt, 1);
			query = "ALTER TABLE transactions ADD " + colName + " INT; ";
			exit = sqlite3_exec(DBhandle, query.c_str(), callback, 0, &messageError);
			//std::cout << colName << std::endl;
		}
		sqlite3_finalize(sqlStmt);

		query = "pragma table_info('transactions');";
		exit = sqlite3_prepare_v2(DBhandle, query.c_str(), query.size() + 1, &sqlStmt, nullptr);
		validatePrepareStmt(exit, DBhandle, query);

		while (sqlite3_step(sqlStmt) != SQLITE_DONE) {
			std::string tableName = (const char*)sqlite3_column_text(sqlStmt, 1);
		}
		sqlite3_finalize(sqlStmt);
		return 0;
	}
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
		component* cmpnt = (component*)malloc(sizeof(component));
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
		while (alpha != nullptr && long(alpha) != long(0xcccccccccccccccc) && long(alpha) != long(0xcdcdcdcdcdcdcdcd)) {
			if (alpha->request->store_id == request->store_id) {
				return -1;
			}
			alpha = alpha->next;
		}
		return 0;
	}

	int enqueue(restock_request* request) {

		if (check_duplicates(request) != 0) return -1;

		Node* NewNode = (Node*)malloc(sizeof(Node));
		NewNode->request = request;
		NewNode->weight = request->weight;

		if (occupied_slots == 0 || long(head) == long(0xcdcdcdcdcdcdcdcd)) {
			head = NewNode;
		}
		else {
			if (NewNode->weight > head->weight) {
				NewNode->next = head;
				head = NewNode;
			}
			else {
				Node* alpha = head;
				while (long(alpha) != long(0xcdcdcdcdcdcdcdcd) && NewNode->weight < alpha->weight) {
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
		if (occupied_slots > 0 || long(head) == long(0xcdcdcdcdcdcdcdcd)) {
			Node* alpha = head;
			restock_request* request = alpha->request;
			head = head->next;
			occupied_slots--;
			free(alpha);
			return request;
		}
	}
	void peek() {
		if (occupied_slots > 0 || long(head) == long(0xcdcdcdcdcdcdcdcd)) {
			component* alpha = head->request->itemListHead;
			while (alpha != nullptr && long(alpha) != long(0xcdcdcdcdcdcdcdcd)) {
				std::cout << "weight | " << head->request->weight << " item id | " << alpha->item_id << " units | " << alpha->units << std::endl;
				alpha = alpha->next;
			}
		}
	}
};

class Warehouse : SQLInventory, SQLDatabase {
	int total_capacity;
	int ClusterSize;
	SQLInventory wrhs_database;
public:

	int process_restock_request(PQueue* Requests, std::recursive_mutex* mutex) {
		if (Requests->occupied_slots == 0) {
			return -1;
		}
		mutex->try_lock();
		restock_request* request = Requests->dequeue();
		mutex->unlock();
		component* alpha = request->itemListHead;
		while (alpha != nullptr && long(alpha) != long(0xcdcdcdcdcdcdcdcd)) {
			wrhs_database.updateUnit("inventory", -(alpha->units), alpha->item_id, true);
			SQLDatabase::showAllRecords("inventory");
			alpha = alpha->next;
		}
		alpha = request->itemListHead;
		while (alpha != nullptr && long(alpha) != long(0xcdcdcdcdcdcdcdcd)) {
			wrhs_database.updateUnit(request->table_name, alpha->units, alpha->item_id, true);
			SQLDatabase::showAllRecords(request->table_name);
			alpha = alpha->next;
		}
		free(request);

		return 0;
	}
};

class Store : SQLDatabase, SQLInventory, SQLTransactions, Warehouse {
	int total_capacity = 2000;
	void compute_restock_request_weight() {
		sqlite3* DBhandle;
		sqlite3_stmt* sqlStmt;
		float threshhold = 0.05;
		std::string query = "SELECT STORENAME,PRICE,UNITS FROM transactions;";

		sqlite3_open(SQLDatabase::wd, &DBhandle);
		int exit = sqlite3_prepare(DBhandle, query.c_str(), query.size() + 1, &sqlStmt, nullptr);
		SQLDatabase::validatePrepareStmt(exit, DBhandle, query);
		float store_total_units = 0;
		float store_total_volume = 0;
		float total_units = 0;
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
		float w = (store_total_units * store_total_volume) / (total_units * total_volume);
		if (w > 0) weight = w;
	}

public:
	int store_id;
	float weight;
	std::string table_name = "";
	std::string name;
	// optional user interface, automatic triggered once a day
	int submit_restock_request(PQueue* Requests, std::recursive_mutex* mutex) {
		restock_request* request = (restock_request*)malloc(sizeof(restock_request));
		request->table_name = (char*)table_name.c_str();
		request->store_id = store_id;
		sqlite3* DBhandle;
		sqlite3_stmt* sqlStmt;
		double threshhold = 0.05;
		std::string query = "SELECT ID, UNITS, CAPACITY FROM " + table_name + ";";
		std::cout << query << std::endl;

		sqlite3_open(SQLDatabase::wd, &DBhandle);
		int exit = sqlite3_prepare(DBhandle, query.c_str(), (int)query.size() + 1, &sqlStmt, nullptr);
		SQLDatabase::validatePrepareStmt(exit, DBhandle, query);
		request->components = 0;
		while (exit = sqlite3_step(sqlStmt) == SQLITE_ROW) {
			int item_id = sqlite3_column_int(sqlStmt, 0);
			double units = sqlite3_column_int(sqlStmt, 1);
			double capacity = sqlite3_column_int(sqlStmt, 2);
			double pStock = units / capacity;
			if (pStock <= threshhold) {
				int requested_units = capacity - units;
				std::cout << pStock << " item id " << item_id << " requested units " << requested_units << std::endl;
				request->CreateComponent(item_id, requested_units);
			}
		}
		sqlite3_finalize(sqlStmt);
		sqlite3_close(DBhandle);
		if (request->components > 0) {
			compute_restock_request_weight();
			request->weight = weight;
			exit = mutex->try_lock();
			Requests->enqueue(request);
			mutex->unlock();
		}
		else {
			free(request);
		}
		return 0;
	}
	int local_transaction(newTransaction* record) {
		SQLTransactions::updateTransactionCols(table_name);
		Tcomponent* alpha = record->itemListHead;
		while (alpha != nullptr) {
			// modify the store inventory
			SQLInventory::updateUnit(table_name, -(alpha->itemunits), alpha->itemId, true);
			SQLTransactions::insertRecord(record->store, record->storeid, record->transactionid, alpha);
			alpha = alpha->next;
		}
		return 0;
	}
};

std::recursive_mutex mutex;

int consumer_thread(Store* store, PQueue* Requests, int multiplier, int freq) {
	SQLDatabase database;

	int count = 0;

	while (true) {
		Tcomponent Tcmpa = { multiplier * 1, 15.00, 2 , "platinum", nullptr };
		newTransaction Transaction = { &Tcmpa, 0, 0, (char*)store->name.c_str() };

		mutex.lock();
		std::cout << store->name << " processing transaction" << std::endl;
		store->local_transaction(&Transaction);
		database.queryDB("SELECT * FROM " + store->table_name, database.callback);
		if (count % 5 == 0 && count >= 5) {
			std::cout << "sending restock request" << std::endl;
			store->submit_restock_request(Requests, &mutex);
			std::cout << "sent restock request" << std::endl;

		}
		mutex.unlock();
		Sleep(1000.0 / freq);
		count++;
	}
	return 0;
}

int supplier_thread(Warehouse* supplier, PQueue* Requests) {
	SQLDatabase database;
	int count = 0;
	while (true) {
		if (Requests->occupied_slots > 0) {
			mutex.lock();
			std::cout << "processing restock request" << std::endl;
			supplier->process_restock_request(Requests, &mutex);
			mutex.unlock();
			std::cout << "processed restock request" << std::endl;

		}
		Sleep(500);

		count++;
	}
	return 0;
}

int main() {
	Warehouse supplier;
	PQueue Requests;
	SQLFacility_Index Index;
	SQLDatabase database;

	newItem itema = { "iron", "metal", 15.00, 1, 250, 0 };
	newItem itemb = { "platinum", "metal", 15.00, 2, 50, 0 };
	newItem itemc = { "aluminum", "metal", 15.00, 3, 250, 0 };
	newFacility storec = { 1111, "storec", "store", "store_inventoryc", "inventory", "transaction", nullptr };
	newFacility storeb = { 420, "storeb", "store", "store_inventoryb", "inventory", "transaction", &storeb };
	newFacility storea = { 1337, "tyrant", "store", "store_inventorya", "inventory", "transaction", &storeb };

	newFacility* head = &storea;

	SQLInventory inventory;
	SQLTransactions TCluster;

	//database.dropTable("facility");
	//Index.createTable("facility");
	
	while (head != nullptr) {
		Index.insertRecord("facility",head);
		head = head->next;
	}
	database.showAllRecords("facility");
	/*
		database.dropTable(store.table_name);
	database.dropTable(storeb.table_name);
	database.dropTable(storec.table_name);

	inventory.createTable(store.table_name);
	inventory.insertRecord(store.table_name, &itemb);
	inventory.createTable(storeb.table_name);
	inventory.insertRecord(storeb.table_name, &itemb);
	inventory.createTable(storec.table_name);
	inventory.insertRecord(storec.table_name, &itemb);

	inventory.updateUnit("inventory", 1000, 2);
	inventory.updateUnit(store.table_name, 20, 2);
	inventory.updateUnit(storeb.table_name, 30, 2);
	inventory.updateUnit(storec.table_name, 10, 2);

	//database.showAllRecords("store_inventory");
	//database.showAllRecords("inventory");

	std::thread storeA(consumer_thread, &store, &Requests, 3, 2);
	std::thread storeB(consumer_thread, &storeb, &Requests, 2, 4);
	std::thread storeC(consumer_thread, &storec, &Requests, 1, 1.7);

	std::thread warehouse(supplier_thread, &supplier, &Requests);
	storeA.join();
	storeB.join();
	storeC.join();
	warehouse.join();
	*/

	return 0;
}