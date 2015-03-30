#include "Model.h"

#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

Model::Model()
{

}

void Model::run(std::string date, std::string session, std::string district)
{
	load(date, session, district);
	std::cout << "Start calculations ... " << std::endl;
	sys.createLinks();
	std::cout << "Link created ... " << std::endl;
	sys.calculateDistribution();
	std::cout << "Calculation Complete" << std::endl;
	save(date, session, district);
	/*std::vector<Link*> test;
	for (auto & garden : sys.gardens)
		if (garden->id == "55")
			for (auto link : garden->links)
				if (link->childrenCount != 0)
					test.push_back(link);*/
}

void Model::load(std::string date, std::string session, std::string district)
{
	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;
		sql::ResultSet *res;

		std::cout << "Connectig" << std::endl;

		driver = get_driver_instance();
		con = driver->connect("151.248.115.206:3306", "Exterior", "9LA0ED7elg3BS3L9");//9LA0ED7elg3BS3L9
		con->setSchema("semantic");
		stmt = con->createStatement();

		std::cout << "Getting data ..." << std::endl;

		std::string query = "SELECT d_houses.id, d_houses.longitude, d_houses.latitude, ";
		query += "(SELECT value FROM d_values WHERE object_id = d_houses.id AND param_id=7 AND `date`='";
		query += date;
		query += "' AND session_id=";
		query += session;
		query += " LIMIT 1) `value`";
		query += "FROM d_houses WHERE district_id=";
		query += district;
		res = stmt->executeQuery(query);

		sys.houses.reserve(res->rowsCount());
		while (res->next())
		{
			if (res->getInt("value") != 0)
				sys.houses.push_back(new House(res->getString("id"),
					Coordinates(res->getDouble("longitude"), res->getDouble("latitude")),
					res->getInt("value")));
		}

		query = "SELECT d_kindergarten.id, d_houses.longitude, d_houses.latitude, ";
		query += "(SELECT value FROM d_values WHERE object_id = d_kindergarten.id AND param_id=1 AND `date`='";
		query += date;
		query += "' AND session_id=";
		query += session;
		query += " LIMIT 1) `value`";
		query += "FROM d_kindergarten INNER JOIN d_houses ON d_houses.id = d_kindergarten.house_id ";
		query += "WHERE (SELECT value FROM d_values ";
		query += "WHERE param_id = 15 AND object_id = d_kindergarten.id AND `date`='";
		query += date;
		query += "' AND session_id = "; 
		query += session;
		query += " LIMIT 1) = ";
		query += district;
		res = stmt->executeQuery(query);

		sys.gardens.reserve(res->rowsCount());
		while (res->next())
		{
			sys.gardens.push_back(new Garden(res->getString("id"),
				Coordinates(res->getDouble("longitude"), res->getDouble("latitude")),
				res->getInt("value")));
		}
		std::cout << "Complete" << std::endl;

		delete res;
		delete stmt;
		delete con;

	}
	catch (sql::SQLException &e) {
		std::cout << "# ERR: SQLException in " << __FILE__;
		std::cout << "(" << __FUNCTION__ << ") on line "
			<< __LINE__ << std::endl;
		std::cout << "# ERR: " << e.what();
		std::cout << " (MySQL error code: " << e.getErrorCode();
		std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
	}
}

void Model::save(std::string date, std::string session, std::string district)
{
	/*for (auto garden : sys.gardens)
		for (auto link : garden.links)
			if (link.childrenCount != 0)
				std::cout << link.house.id << " => " << link.garden.id << " = " << link.childrenCount << std::endl;

	std::cout << std::endl;
	for (auto house : sys.houses)
		std::cout << "House: " << house.id
		<< "\n\t Average distance " << house.weightedAvgDistance
		<< "\n\t Average distance uniformity " << house.weightedAvgDistanceUniformity << std::endl;
	std::cout << std::endl;
	for (auto garden : sys.gardens)
		std::cout << "Garden: " << garden.id
		<< "\n\t Average distance " << garden.weightedAvgDistance
		<< "\n\t Average distance uniformity " << garden.weightedAvgDistanceUniformity << std::endl;

	std::cout << "System: "
		<< "\n\t Sufficiency " << sys.sufficiency
		<< "\n\t Average distance " << sys.weightedAvgDistance
		<< "\n\t Average distance uniformity " << sys.weightedAvgDistanceUniformity << std::endl;*/

	std::cout << "Connecting ..." << std::endl;
	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;
		sql::ResultSet *res = nullptr;
		sql::PreparedStatement  *prepStmt;

		driver = get_driver_instance();
		con = driver->connect("151.248.115.206:3306", "Exterior", "9LA0ED7elg3BS3L9"); //9LA0ED7elg3BS3L9
		con->setSchema("semantic");
		stmt = con->createStatement();

		std::cout << "Sending data ..." << std::endl;

		std::string paramId = "25";
		stmt->executeUpdate("DELETE FROM d_values WHERE param_id=" + paramId + " AND date = '" + date + "' AND session_id = '" + session + "'");
		prepStmt = con->prepareStatement("INSERT INTO d_values(object_id, param_id, value, date, session_id) VALUES (?, ?, ?, ?, ?)");
		for (auto house : sys.houses)
		{
			prepStmt->setString(1, house->id);
			prepStmt->setString(2, paramId);
			prepStmt->setDouble(3, house->sufficiency);
			prepStmt->setString(4, date);
			prepStmt->setString(5, session);
			prepStmt->execute();
		}

		paramId = "29";
		stmt->executeUpdate("DELETE FROM d_values WHERE param_id=" + paramId + " AND date='" + date + "' AND session_id='" + session + "'");
		prepStmt = con->prepareStatement("INSERT INTO d_values(object_id, param_id, value, date, session_id) VALUES (?, ?, ?, ?, ?)");
		for (auto house : sys.houses)
		{
			prepStmt->setString(1, house->id);
			prepStmt->setString(2, paramId);
			prepStmt->setDouble(3, house->weightedAvgDistance);
			prepStmt->setString(4, date);
			prepStmt->setString(5, session);
			prepStmt->execute();
		}

		paramId = "33";
		stmt->executeUpdate("DELETE FROM d_values WHERE param_id=" + paramId + " AND date='" + date + "' AND session_id='" + session + "'");
		prepStmt = con->prepareStatement("INSERT INTO d_values(object_id, param_id, value, date, session_id) VALUES (?, ?, ?, ?, ?)");
		for (auto house : sys.houses)
		{
			prepStmt->setString(1, house->id);
			prepStmt->setString(2, paramId);
			prepStmt->setDouble(3, probaFromDistance(house->weightedAvgDistance));
			prepStmt->setString(4, date);
			prepStmt->setString(5, session);
			prepStmt->execute();
		}

		paramId = "30";
		stmt->executeUpdate("DELETE FROM d_values WHERE param_id=" + paramId + " AND date='" + date + "' AND session_id='" + session + "'");
		prepStmt = con->prepareStatement("INSERT INTO d_values(object_id, param_id, value, date, session_id) VALUES (?, ?, ?, ?, ?)");
		for (auto house : sys.houses)
		{
			prepStmt->setString(1, house->id);
			prepStmt->setString(2, paramId);
			prepStmt->setDouble(3, house->weightedAvgDistanceUniformity);
			prepStmt->setString(4, date);
			prepStmt->setString(5, session);
			prepStmt->execute();
		}

		paramId = "34";
		stmt->executeUpdate("DELETE FROM d_values WHERE param_id=" + paramId + " AND date='" + date + "' AND session_id='" + session + "'");
		prepStmt = con->prepareStatement("INSERT INTO d_values(object_id, param_id, value, date, session_id) VALUES (?, ?, ?, ?, ?)");
		for (auto house : sys.houses)
		{
			prepStmt->setString(1, house->id);
			prepStmt->setString(2, paramId);
			prepStmt->setDouble(3, probaFromDistance(house->weightedAvgDistanceUniformity));
			prepStmt->setString(4, date);
			prepStmt->setString(5, session);
			prepStmt->execute();
		}

		paramId = "27";
		stmt->executeUpdate("DELETE FROM d_values WHERE param_id=" + paramId + " AND date='" + date + "' AND session_id='" + session + "'");
		prepStmt = con->prepareStatement("INSERT INTO d_values(object_id, param_id, value, date, session_id) VALUES (?, ?, ?, ?, ?)");
		for (auto garden : sys.gardens)
		{
			prepStmt->setString(1, garden->id);
			prepStmt->setString(2, paramId);
			prepStmt->setDouble(3, garden->weightedAvgDistance);
			prepStmt->setString(4, date);
			prepStmt->setString(5, session);
			prepStmt->execute();
		}

		paramId = "35";
		stmt->executeUpdate("DELETE FROM d_values WHERE param_id=" + paramId + " AND date='" + date + "' AND session_id='" + session + "'");
		prepStmt = con->prepareStatement("INSERT INTO d_values(object_id, param_id, value, date, session_id) VALUES (?, ?, ?, ?, ?)");
		for (auto garden : sys.gardens)
		{
			prepStmt->setString(1, garden->id);
			prepStmt->setString(2, paramId);
			prepStmt->setDouble(3, probaFromDistance(garden->weightedAvgDistance));
			prepStmt->setString(4, date);
			prepStmt->setString(5, session);
			prepStmt->execute();
		}

		paramId = "28";
		stmt->executeUpdate("DELETE FROM d_values WHERE param_id=" + paramId + " AND date='" + date + "' AND session_id='" + session + "'");
		prepStmt = con->prepareStatement("INSERT INTO d_values(object_id, param_id, value, date, session_id) VALUES (?, ?, ?, ?, ?)");
		for (auto garden : sys.gardens)
		{
			prepStmt->setString(1, garden->id);
			prepStmt->setString(2, paramId);
			prepStmt->setDouble(3, garden->weightedAvgDistanceUniformity);
			prepStmt->setString(4, date);
			prepStmt->setString(5, session);
			prepStmt->execute();
		}

		paramId = "36";
		stmt->executeUpdate("DELETE FROM d_values WHERE param_id=" + paramId + " AND date='" + date + "' AND session_id='" + session + "'");
		prepStmt = con->prepareStatement("INSERT INTO d_values(object_id, param_id, value, date, session_id) VALUES (?, ?, ?, ?, ?)");
		for (auto garden : sys.gardens)
		{
			prepStmt->setString(1, garden->id);
			prepStmt->setString(2, paramId);
			prepStmt->setDouble(3, probaFromDistance(garden->weightedAvgDistanceUniformity));
			prepStmt->setString(4, date);
			prepStmt->setString(5, session);
			prepStmt->execute();
		}

		paramId = "32";
		stmt->executeUpdate("DELETE FROM d_values WHERE param_id=" + paramId + " AND date='" + date + "' AND session_id='" + session + "'");
		prepStmt = con->prepareStatement("INSERT INTO d_values(object_id, param_id, value, date, session_id) VALUES (?, ?, ?, ?, ?)");
		prepStmt->setString(1, district);
		prepStmt->setString(2, paramId);
		prepStmt->setDouble(3, sys.sufficiency);
		prepStmt->setString(4, date);
		prepStmt->setString(5, session);
		prepStmt->execute();

		paramId = "31";
		stmt->executeUpdate("DELETE FROM d_values WHERE param_id=" + paramId + " AND date='" + date + "' AND session_id='" + session + "'");
		prepStmt = con->prepareStatement("INSERT INTO d_values(object_id, param_id, value, date, session_id) VALUES (?, ?, ?, ?, ?)");
		prepStmt->setString(1, district);
		prepStmt->setString(2, paramId);
		prepStmt->setDouble(3, sys.weightedAvgDistance);
		prepStmt->setString(4, date);
		prepStmt->setString(5, session);
		prepStmt->execute();

		paramId = "37";
		stmt->executeUpdate("DELETE FROM d_values WHERE param_id=" + paramId + " AND date='" + date + "' AND session_id='" + session + "'");
		prepStmt = con->prepareStatement("INSERT INTO d_values(object_id, param_id, value, date, session_id) VALUES (?, ?, ?, ?, ?)");
		prepStmt->setString(1, district);
		prepStmt->setString(2, paramId);
		prepStmt->setDouble(3, probaFromDistance(sys.weightedAvgDistance));
		prepStmt->setString(4, date);
		prepStmt->setString(5, session);
		prepStmt->execute();

		paramId = "26";
		stmt->executeUpdate("DELETE FROM d_values WHERE param_id=" + paramId + " AND date='" + date + "' AND session_id='" + session + "'");
		prepStmt = con->prepareStatement("INSERT INTO d_values(object_id, param_id, value, date, session_id) VALUES (?, ?, ?, ?, ?)");
		prepStmt->setString(1, district);
		prepStmt->setString(2, paramId);
		prepStmt->setDouble(3, sys.weightedAvgDistanceUniformity);
		prepStmt->setString(4, date);
		prepStmt->setString(5, session);
		prepStmt->execute();

		paramId = "38";
		stmt->executeUpdate("DELETE FROM d_values WHERE param_id=" + paramId + " AND date='" + date + "' AND session_id='" + session + "'");
		prepStmt = con->prepareStatement("INSERT INTO d_values(object_id, param_id, value, date, session_id) VALUES (?, ?, ?, ?, ?)");
		prepStmt->setString(1, district);
		prepStmt->setString(2, paramId);
		prepStmt->setDouble(3, probaFromDistance(sys.weightedAvgDistanceUniformity));
		prepStmt->setString(4, date);
		prepStmt->setString(5, session);
		prepStmt->execute();

		std::cout << "Complete" << std::endl;

		delete prepStmt;
		delete res;
		delete stmt;
		delete con;

	}
	catch (sql::SQLException &e) {
		std::cout << "# ERR: SQLException in " << __FILE__;
		std::cout << "(" << __FUNCTION__ << ") on line "
			<< __LINE__ << std::endl;
		std::cout << "# ERR: " << e.what();
		std::cout << " (MySQL error code: " << e.getErrorCode();
		std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
	}

}