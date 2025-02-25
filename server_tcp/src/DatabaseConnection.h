#pragma once
#include <stdint.h>
#include <string>
#include <chrono>
#include <libpq-fe.h>
#include "Student.h"

namespace nameplate
{

class DatabaseConnection
{
public:
	DatabaseConnection(const char* databaseName, const char* username, const char* password);
	DatabaseConnection(const DatabaseConnection&) = delete;
	~DatabaseConnection();

	DatabaseConnection operator=(const DatabaseConnection&) = delete;

public:
	bool DoesStudentExist(uint32_t id);
	void CreateStudent(uint32_t id, const char* lastName, const char* firstName);
	void RecordAttendance(uint32_t id, std::chrono::system_clock::time_point time);
	Student FetchStudent(uint32_t id);

private:
	bool Query(const std::string& query);

private:
	PGconn* m_connection = nullptr;
	PGresult* m_lastResult = nullptr;
};

}