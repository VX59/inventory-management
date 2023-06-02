#define FMT_HEADER_ONLY
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <sqlite3.h>
#include <string>
#include <fmt/core.h>

typedef struct Item {
	const char* NAME;
	const char* CATEGORY;
	float PRICE;
	const int ID;
	float CAPACITY;
	float UNITS;
};

class SQLDatabase {
private:	
	const char* wd = "C:\Users\deros\Documents\inventory";
	static int callback(void* NotUsed, int argc, char** argv, char** asColName) {
		for (int i = 0; i < argc; i++) {
			std::cout << fmt::format("|{}: {:<{}}", asColName[i], argv[i], 14);
		}
		std::cout << std::endl;
		return 0;
	}
	int queryDB(std::string query, static int (*cb)(void* NotUsed, int argc, char** argv, char** asColName), char* messageError) {
		sqlite3* DBhandle;
		int exit = sqlite3_open(wd, &DBhandle);
		exit = sqlite3_exec(DBhandle, query.c_str(), cb, 0, &messageError);
		return exit;
	}
	float isNumber(const char* input) {
		for (int i = 0; input[i] != '\0'; i++) {
			if (!(isdigit(input[i])) && input[i] != '.') return -1;
		}
		return std::stof(input);
	}
public: 
	int createDB() {
		sqlite3* DBhandle;
		sqlite3_open(wd, &DBhandle);
		sqlite3_close(DBhandle);
		return 0;
	}

	int createTable() {
		sqlite3* DBhandle;
		std::string query = "CREATE TABLE items ("
			"ID integer PRIMARY KEY,"
			"NAME TEXT,"
			"UNITS INT,"
			"PRICE INT,"
			"CAPACITY INT,"
			"CATEGORY TEXT)";

		try {
			char* messageError = (char*)malloc(128);
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

	int insertRecord(Item *record) {
		char* messageError;
		sqlite3* DBhandle;
		int exit = sqlite3_open(wd, &DBhandle);
		sqlite3_stmt* sqlStmt = nullptr;
		std::string query = "INSERT INTO items (ID, NAME, UNITS, PRICE, CAPACITY, CATEGORY) VALUES( ?1, ?2, ?3, ?4, ?5, ?6);";
		exit = sqlite3_prepare_v2(DBhandle, query.c_str(), query.size() + 1, &sqlStmt, nullptr);
		if (exit != SQLITE_OK) {
			sqlite3_close(DBhandle);
			std::cout << "failed to prepare statment";
			return -1;
		}
		sqlite3_bind_int(sqlStmt, 1, record->ID);
		sqlite3_bind_text(sqlStmt, 2, record->NAME, -1, NULL);
		sqlite3_bind_int(sqlStmt, 3, record->UNITS);
		sqlite3_bind_int(sqlStmt, 4, record->PRICE);
		sqlite3_bind_int(sqlStmt, 5, record->CAPACITY);
		sqlite3_bind_text(sqlStmt, 6, record->CATEGORY, -1, NULL);

		exit = sqlite3_step(sqlStmt);
		if(exit != SQLITE_DONE) {
			sqlite3_close(DBhandle);
			std::cout << "failed to execute query";
			return -1;
		}
		sqlite3_finalize(sqlStmt);

		return 0;
	}

	int showAllRecords() {
		std::string query = "SELECT * FROM items;";
		int exit = queryDB(query, callback, NULL);
		return 0;
	}

	int updateUnit(float newval, const char* condval) {
		char* messageError;
		sqlite3* DBhandle;
		int exit = sqlite3_open(wd, &DBhandle);
		sqlite3_stmt* sqlStmt = nullptr;
		std::string query = "UPDATE items SET UNITS = ?2 WHERE NAME = ?4;";
		exit = sqlite3_prepare_v2(DBhandle, query.c_str(), query.size() + 1, &sqlStmt, nullptr);
		if (exit != SQLITE_OK) {
			sqlite3_close(DBhandle);
			std::cout << "failed to prepare statment";
			return -1;
		}

		sqlite3_bind_int(sqlStmt, 2, newval);
		sqlite3_bind_text(sqlStmt, 4, condval, -1, NULL);

		exit = sqlite3_step(sqlStmt);
		if (exit != SQLITE_DONE) {
			sqlite3_close(DBhandle);
			std::cout << "Error updating record";
			return -1;
		}
		sqlite3_finalize(sqlStmt);
		return 0;
	}

	int dropTable() {
		char* messageError = (char*)malloc(1024);
		std::string query("DROP TABLE items;");
		int exit = queryDB(query, callback, messageError);
		return 0;
	}
};

int main() {
	SQLDatabase Inventory;
	Item item = { "weed", "drug", 7.00, 3, 1000, 31.1 };
	//Inventory.createTable();
	//Inventory.insertRecord(&item);
	//Inventory.showAllRecords();
	Inventory.updateUnit(100, "weed");
	Inventory.showAllRecords();
	//Inventory.dropTable();
	return 0;
}