#include <string.h>
#include <ctime>
#include <sstream>
#include <stdexcept>


#define SCROLLS_USE_LOGGER_MACROS
#include <Scrolls.h>

#include "DatabaseConnection.h"

namespace nameplate
{

DatabaseConnection::DatabaseConnection(const char* databaseName, const char* user, const char* password)
{
	constexpr size_t CON_INFO_SZ = 512;
	char connectionInfo[CON_INFO_SZ];

	snprintf(connectionInfo, CON_INFO_SZ, "dbname=%s user=%s password=%s", databaseName, user, password);

	m_connection = PQconnectdb(connectionInfo);

	if (PQstatus(m_connection) != CONNECTION_OK)
	{
		ERROR("[Database] Error connecting to database: " + std::string(PQerrorMessage(m_connection)));
		PQfinish(m_connection);
		throw std::runtime_error("fatal database error");
	}
}

DatabaseConnection::~DatabaseConnection()
{
	if (m_connection)
	{
		PQfinish(m_connection);
	}

	if (m_lastResult)
	{
		PQclear(m_lastResult);
	}
}

bool DatabaseConnection::Query(const std::string& query)
{
	if (m_lastResult)
	{
		PQclear(m_lastResult);
		m_lastResult = nullptr;
	}

	LOG_DEBUG("[Database] Query: " + query);
	m_lastResult = PQexec(m_connection, query.c_str());

	const ExecStatusType status = PQresultStatus(m_lastResult);
	if (status != PGRES_TUPLES_OK && status != PGRES_COMMAND_OK)
	{
		ERROR("[Database] Query failed to execute: " + std::string(PQerrorMessage(m_connection)));
		return false;
	}

	return true;
}

bool DatabaseConnection::DoesStudentExist(uint32_t id)
{
	const std::string query = "SELECT * FROM student WHERE student_id=" + std::to_string(id) + ';';
	
	if (!Query(query))
	{
		return false;
	}

	const int numRows = PQntuples(m_lastResult);

	return numRows > 0;
}

void DatabaseConnection::CreateStudent(uint32_t id, const char* lastName, const char* firstName)
{
	std::stringstream query;
	query << "INSERT INTO student VALUES (";
	query << id << ", '" << lastName << "', '" << firstName << "');";

	Query(query.str());
}

void DatabaseConnection::RecordAttendance(uint32_t id, std::chrono::system_clock::time_point time)
{
	std::stringstream query;

	const std::time_t timeNow = std::chrono::system_clock::to_time_t(time);
	const std::tm* localTime = std::localtime(&timeNow);

	const int year = localTime->tm_year + 1900;
	const int month = localTime->tm_mon + 1;
	const int day = localTime->tm_mday;
	const int hour = localTime->tm_hour;
	const int minute = localTime->tm_min;
	const int second = localTime->tm_sec;


	query << "INSERT INTO attendance (student_id, attendance_date, arrivial_time) VALUES (";
	query << id << ", '" << year << '-' << month << '-' << day << "', '"; 
	query << hour << ':' << minute << ':' << second << "');";

	Query(query.str());
}

Student DatabaseConnection::FetchStudent(uint32_t id)
{

	if (!DoesStudentExist(id))
	{
		return { };
	}

	const std::string query = "SELECT * FROM student WHERE student_id=" + std::to_string(id) + ';';

	Query(query);

	const char* studentIdStr = PQgetvalue(m_lastResult, 0, 0);
	const char* studentLastName = PQgetvalue(m_lastResult, 0, 1);
	const char* studentFirstName = PQgetvalue(m_lastResult, 0, 2);

	const unsigned int studentId = atoi(studentIdStr);

	return { studentId, studentLastName, studentFirstName };
}



}
