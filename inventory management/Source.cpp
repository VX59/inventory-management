#include "setup.h"
#include "database_api.cpp"

class Warehouse : SQLInventory, SQLDatabase {
	int total_capacity;
	int ClusterSize;
	SQLInventory wrhs_database;
public:
	int longitude, laditude;
	int process_restock_request(PQueue* Requests, std::recursive_mutex* mutex) {
		if (Requests->occupied_slots == 0) {
			return -1;
		}

		restock_request* request = Requests->dequeue();
		component* alpha = request->itemListHead;
		while (alpha != nullptr) {
			wrhs_database.updateUnit("inventory", -(alpha->units), alpha->item_id, true);
			SQLDatabase::showAllRecords("inventory");
			alpha = alpha->next;
		}
		alpha = request->itemListHead;
		while (alpha != nullptr) {
			wrhs_database.updateUnit(request->table_name, alpha->units, alpha->item_id, true);
			SQLDatabase::showAllRecords(request->table_name);
			alpha = alpha->next;
		}
		return 0;
	}
};

class Store : SQLDatabase, SQLInventory, SQLTransactions, Warehouse {
	int total_capacity = 2000;
	int longitude, laditude;

	void compute_restock_request_weight() {
		sqlite3* DBhandle;
		sqlite3_stmt* sqlStmt;
		float threshhold = 0.05;
		std::string query = "SELECT STORENAME,PRICE,UNITS FROM transactions;";

		sqlite3_open(SQLDatabase::wd, &DBhandle);
		int exit = sqlite3_prepare(DBhandle, query.c_str(), query.size() + 1, &sqlStmt, nullptr);
		SQLDatabase::validatePrepareStmt(exit, DBhandle);
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
		restock_request* request = new restock_request();
		request->table_name = (char*)table_name.c_str();
		request->store_id = store_id;
		sqlite3* DBhandle;
		sqlite3_stmt* sqlStmt;
		double threshhold = 0.05;
		std::string query = "SELECT ID, UNITS, CAPACITY FROM " + table_name + ";";
		std::cout << query << std::endl;

		sqlite3_open(SQLDatabase::wd, &DBhandle);
		int exit = sqlite3_prepare(DBhandle, query.c_str(), (int)query.size() + 1, &sqlStmt, nullptr);
		SQLDatabase::validatePrepareStmt(exit, DBhandle);
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
			Requests->enqueue(request);
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

	while (count < 150) {
		Tcomponent Tcmpa = { multiplier * 1, 15.00, 2 , "platinum", nullptr };
		newTransaction Transaction = { &Tcmpa, 0, 0, (char*)store->name.c_str() };

		mutex.lock();
		std::cout << "processing transaction" << std::endl;
		store->local_transaction(&Transaction);
		database.queryDB("SELECT * FROM " + store->table_name, database.callback);
		if (count % 5 == 0 && count >= 5) {
			std::cout << "sending restock request" << std::endl;
			store->submit_restock_request(Requests, &mutex);
			std::cout << "sent restock request" << std::endl;

		}
		mutex.unlock();
		Sleep(50 / freq);
		count++;
	}
	return 0;
}

int supplier_thread(Warehouse* supplier, PQueue* Requests) {
	SQLDatabase database;
	int count = 0;
	while (count < 150) {
		if (Requests->occupied_slots > 0) {
			
			std::cout << "processing restock request" << std::endl;
			mutex.lock();
			supplier->process_restock_request(Requests, &mutex);
			mutex.unlock();
			std::cout << "processed restock request" << std::endl;

		}
		Sleep(100);

		count++;
	}
	return 0;
}

int main() {
	Warehouse supplier;
	PQueue Requests;
	SQLDatabase database;

	newItem itema = { "iron", "metal", 15.00, 1, 250, 0 };
	newItem itemb = { "platinum", "metal", 15.00, 2, 50, 0 };
	newItem itemc = { "aluminum", "metal", 15.00, 3, 250, 0 };

	Store store;
	store.store_id = 1337;
	store.name = "tyrant";
	store.table_name = "store_inventorya";

	Store storeb;
	storeb.store_id = 420;
	storeb.name = "storeb";
	storeb.table_name = "store_inventoryb";

	Store storec;
	storec.store_id = 234;
	storec.name = "storec";
	storec.table_name = "store_inventoryc";

	SQLInventory inventory;
	SQLTransactions TCluster;

	//inventory.updateCapacity("store_inventory", 50, 2);
	//database.dropTable("store_inventory");

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
	inventory.updateUnit(storec.table_name, 40, 2);


	//database.showAllRecords("store_inventory");
	//database.showAllRecords("inventory");

	std::thread storeA(consumer_thread, &store, &Requests, 3, 2);
	std::thread storeB(consumer_thread, &storeb, &Requests, 2, 4);
	std::thread storeC(consumer_thread, &storec, &Requests, 1, 1);

	std::thread warehouse(supplier_thread, &supplier, &Requests);

	warehouse.join();
	storeA.join();
	storeB.join();
	storeC.join();
	return 0;
}