#include <string.h>
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
		std::runtime_error("fatal database error");
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
	LOG_DEBUG("[Database] Query: " + query);
	
	if (!Query(query))
	{
		return false;
	}

	const int numRows = PQntuples(m_lastResult);

	return numRows > 0;
}

}
