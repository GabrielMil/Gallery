#include "DatabaseAccess.h"

bool DatabaseAccess::open()
{
	std::string sqlStatement;
	char* errMessage;
	int res = 0;

	//Make sure foreign-keys enabled
	sqlStatement = "PRAGMA FOREIGN_KEYS=true;";
	errMessage = nullptr;
	res = sqlite3_exec(this->db, sqlStatement.c_str(), nullptr, nullptr, &errMessage);
	if (res != SQLITE_OK)
	{
		std::cout << errMessage << std::endl;
		return false;
	}
	
	//Users
	sqlStatement = "CREATE TABLE IF NOT EXISTS Users(Id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,Name text NOT NULL);";
	errMessage = nullptr;
	res = sqlite3_exec(this->db, sqlStatement.c_str(), nullptr, nullptr, &errMessage);
	if (res != SQLITE_OK)
	{
		std::cout << errMessage << std::endl;
		return false;
	}

	//Albums
	sqlStatement = "CREATE TABLE IF NOT EXISTS Albums(Id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,Name text NOT NULL,Creation_Date text NOT NULL,User_Id intEGER NOT NULL,FOREIGN KEY(User_Id) REFERENCES Users(Id) ON DELETE CASCADE ON UPDATE CASCADE);";
	errMessage = nullptr;
	res = sqlite3_exec(this->db, sqlStatement.c_str(), nullptr, nullptr, &errMessage);
	if (res != SQLITE_OK)
	{
		std::cout << errMessage << std::endl;
		return false;
	}

	//Pictures
	sqlStatement = "CREATE TABLE IF NOT EXISTS Pictures(Id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,Name text NOT NULL,Location text NOT NULL,Creation_Date text NOT NULL,Album_Id intEGER NOT NULL,FOREIGN KEY(Album_Id) REFERENCES Albums(Id) ON DELETE CASCADE ON UPDATE CASCADE);";
	errMessage = nullptr;
	res = sqlite3_exec(this->db, sqlStatement.c_str(), nullptr, nullptr, &errMessage);
	if (res != SQLITE_OK)
	{
		std::cout << errMessage << std::endl;
		return false;
	}

	//Tags
	sqlStatement = "CREATE TABLE IF NOT EXISTS Tags(Id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,Picture_Id intEGER NOT NULL,User_Id intEGER NOT NULL,FOREIGN KEY(Picture_Id) REFERENCES Pictures(Id) ON DELETE CASCADE ON UPDATE CASCADE,FOREIGN KEY(User_Id) REFERENCES Users(Id) ON DELETE CASCADE ON UPDATE CASCADE);";
	errMessage = nullptr;
	res = sqlite3_exec(this->db, sqlStatement.c_str(), nullptr, nullptr, &errMessage);
	if (res != SQLITE_OK)
	{
		std::cout << errMessage << std::endl;
		return false;
	}


	return true;
}

void DatabaseAccess::close()
{}

void DatabaseAccess::clear()
{}

void DatabaseAccess::execQuery(std::string & sqlStatement, int(*callBack)(void* givenParam, int argc, char** argv, char** azColName), void* param)
{
	//Helping Function
	char* errMessage = nullptr;
	int res = 0;

	res = sqlite3_exec(this->db, sqlStatement.c_str(), callBack, param, &errMessage);
	if (res != SQLITE_OK)
	{
		db = nullptr;
		throw std::exception(errMessage);
	}
}

std::list<Picture> DatabaseAccess::getPicturesOfAlbumFromDB(const Album album)
{
	std::list<Picture> pictureList;
	std::string sqlStatement = "SELECT * FROM pictures WHERE album_id=(SELECT albums.id FROM albums WHERE name='" + album.getName() + "');";
	execQuery(sqlStatement, CallBackFunctions::createPicture, &pictureList);
	return pictureList;
}

void DatabaseAccess::createCompletedPictures(std::list<Picture>& pictureList)
{
	std::string sqlStatement = "";
	std::list<int> tags;
	//Fill the pictures with tags
	for (Picture& picture : pictureList)
	{
		sqlStatement = "SELECT user_id FROM tags WHERE picture_id=" + std::to_string(picture.getId()) + ";";
		execQuery(sqlStatement, CallBackFunctions::getNumbers, &tags);
		for (int& userId : tags)
		{
			picture.tagUser(userId);
		}
		tags.clear();  // clear the old tags
	}
}

DatabaseAccess::DatabaseAccess()
{
	this->dbFileName = "MyDB.sqlite";
	int res = sqlite3_open(dbFileName.c_str(), &(this->db));
	if (res != SQLITE_OK)
	{
		db = nullptr;
		throw std::exception("Failed to open DB!");
	}
}

DatabaseAccess::~DatabaseAccess()
{
	sqlite3_close(this->db);
	this->db = nullptr;
}

const std::list<Album> DatabaseAccess::getAlbums()
{
	std::list<Album> albumList;
	std::string sqlStatement = "SELECT * FROM albums;";
	execQuery(sqlStatement, CallBackFunctions::createAlbum, &albumList);
	return albumList;
}

const std::list<Album> DatabaseAccess::getAlbumsOfUser(const User& user)
{
	std::list<Album> albumList;
	std::string sqlStatement = "SELECT * FROM albums WHERE user_id=" + std::to_string(user.getId()) + ";";
	execQuery(sqlStatement, CallBackFunctions::createAlbum, &albumList);
	return albumList;
}

void DatabaseAccess::createAlbum(const Album& album)
{
	std::string sqlStatement = "INSERT INTO albums(name, user_id, creation_date) VALUES('" +
		album.getName() + "', " + std::to_string(album.getOwnerId()) + ", '" + album.getCreationDate() + "');";
	execQuery(sqlStatement);
}

void DatabaseAccess::deleteAlbum(const std::string& albumName, int userId)
{
	std::string sqlStatement = "DELETE FROM albums WHERE name='" + albumName + "' AND user_id=" + std::to_string(userId) + ";";
	execQuery(sqlStatement);
}

bool DatabaseAccess::doesAlbumExists(const std::string& albumName, int userId)
{
	int count = 0;
	std::string sqlStatement = "SELECT * FROM albums WHERE name='" + albumName + "' AND user_id=" + std::to_string(userId) + ";";
	execQuery(sqlStatement, CallBackFunctions::countRows, &count);
	return count > 0;  // if count increased it means that the album exist
}

Album DatabaseAccess::openAlbum(const std::string& albumName)
{
	std::list<Album> albumList;
	std::list<Picture> pictureList;
	Album currAlbum;

	//Get the album
	std::string sqlStatement = "SELECT * FROM albums WHERE name='" + albumName + "';";
	execQuery(sqlStatement, CallBackFunctions::createAlbum, &albumList);
	if (albumList.empty())  // Check if album exist
	{
		throw std::exception("Album not exist");
	}
	currAlbum = albumList.front();
	//Fill pictures with tags
	pictureList = getPicturesOfAlbumFromDB(currAlbum);
	createCompletedPictures(pictureList);
	//Fill album with pictures
	for (const Picture picture : pictureList)
	{
		currAlbum.addPicture(picture);
	}
	return currAlbum;
}

void DatabaseAccess::closeAlbum(Album& pAlbum)
{
	pAlbum = Album();
}

void DatabaseAccess::printAlbums()
{
	std::list<Album> albumList = getAlbums();
	std::cout << "All Albums:" << std::endl;
	std::list<Album>::iterator albumIter = albumList.begin();
	while (albumIter != albumList.end())
	{
		std::cout << (*albumIter) << std::endl;
		++albumIter;
	}
}

void DatabaseAccess::addPictureToAlbumByName(const std::string& albumName, const Picture& picture)
{
	//Insert into sql
	int lastId = 0;
	std::string sqlStatement = "INSERT INTO pictures(name, location, creation_date, album_id) VALUES('";
	sqlStatement += picture.getName() + "', '" + picture.getPath() + "', '" + picture.getCreationDate() + "', (" +
		"SELECT albums.id FROM albums WHERE name='" + albumName + "'));";
	execQuery(sqlStatement);
	//Print
	sqlStatement = "SELECT id FROM pictures ORDER BY id DESC LIMIT 1;";
	execQuery(sqlStatement, CallBackFunctions::getSingleNum, &lastId);
	std::cout << "Picture [" << lastId << "] successfully added to Album [" << albumName << "]." << std::endl;
}

void DatabaseAccess::removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName)
{
	std::string sqlStatement = "DELETE FROM pictures WHERE name=" + pictureName + " AND album_id=(" +
		"SELECT albums.id FROM albums WHERE name='" + albumName + "');";
	execQuery(sqlStatement);
}

void DatabaseAccess::tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	std::string sqlStatement = "INSERT INTO tags(picture_id, user_id) VALUES((SELECT id FROM pictures WHERE name='" + pictureName + "' AND album_id=(SELECT albums.id FROM albums WHERE name='" + albumName + "')), " + std::to_string(userId) + ");";
	execQuery(sqlStatement);
}

void DatabaseAccess::untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	std::string sqlStatement = "DELETE FROM tags WHERE user_id=" + std::to_string(userId) + " AND picture_id = (SELECT id FROM pictures WHERE name = '" + pictureName + "' AND album_id = (SELECT albums.id FROM albums WHERE name = '" + albumName + "'));";
	execQuery(sqlStatement);
}

void DatabaseAccess::printUsers()
{
	std::list<User> userList;
	std::string sqlStatement = "SELECT * FROM users;";
	execQuery(sqlStatement, CallBackFunctions::createUser, &userList);
	std::cout << "All Users:" << std::endl;
	std::list<User>::iterator userIter = userList.begin();
	while (userIter != userList.end())
	{
		std::cout << (*userIter) << std::endl;
		++userIter;
	}
}

void DatabaseAccess::createUser(User& user)
{
	int lastId = 0;
	//Insert into sql
	std::string sqlStatement = "INSERT INTO users(name) VALUES('" + user.getName() + "');";
	execQuery(sqlStatement);
	//Print
	sqlStatement = "SELECT id FROM users ORDER BY id DESC LIMIT 1;";
	execQuery(sqlStatement, CallBackFunctions::getSingleNum, &lastId);
	std::cout << "User " << user.getName() << " with id @" << lastId << " created successfully." << std::endl;
}

void DatabaseAccess::deleteUser(const User& user)
{
	std::string sqlStatement = "DELETE FROM users WHERE id=" + std::to_string(user.getId()) + ";";
	execQuery(sqlStatement);
}

bool DatabaseAccess::doesUserExists(int userId)
{
	int count = 0;
	std::string sqlStatement = "SELECT * FROM users WHERE id=" + std::to_string(userId) + ";";
	execQuery(sqlStatement, CallBackFunctions::countRows, &count);
	return count > 0;  // if count increased it means that the user exist
}

User DatabaseAccess::getUser(int userId)
{
	std::list<User> userList;
	std::string sqlStatement = "SELECT * FROM users WHERE id=" + std::to_string(userId) + ";";
	execQuery(sqlStatement, CallBackFunctions::createUser, &userList);
	return (userList.size() > 0) ? userList.front() : throw std::exception("User not exist");
}

int DatabaseAccess::countAlbumsOwnedOfUser(const User& user)
{
	int count = 0;
	std::string sqlStatement = "SELECT COUNT(id) FROM albums WHERE user_id=" + std::to_string(user.getId()) + ";";
	execQuery(sqlStatement, CallBackFunctions::getSingleNum, &count);
	return count;
}

int DatabaseAccess::countAlbumsTaggedOfUser(const User& user)
{
	int count = 0;
	std::string sqlStatement = "SELECT COUNT(DISTINCT album_id) FROM pictures JOIN tags ON pictures.id=tags.picture_id WHERE tags.user_id=" + std::to_string(user.getId()) + ";";
	execQuery(sqlStatement, CallBackFunctions::getSingleNum, &count);
	return count;
}

int DatabaseAccess::countTagsOfUser(const User& user)
{
	int count = 0;
	std::string sqlStatement = "SELECT COUNT(user_id) FROM tags WHERE user_id=" + std::to_string(user.getId()) + ";";
	execQuery(sqlStatement, CallBackFunctions::getSingleNum, &count);
	return count;
}

float DatabaseAccess::averageTagsPerAlbumOfUser(const User& user)
{
	int totalTagsNum = countTagsOfUser(user);
	int amountOfAlbumsNum = countAlbumsTaggedOfUser(user);
	if (totalTagsNum == 0)  //Check if user has any tags
	{
		return 0;
	}
	return (static_cast<float>(totalTagsNum)) / amountOfAlbumsNum;
}

User DatabaseAccess::getTopTaggedUser()
{
	std::list<User> userList;
	std::string sqlStatement = "SELECT * FROM Users WHERE Id=(SELECT User_Id FROM Tags GROUP BY User_Id ORDER BY COUNT(User_Id) DESC LIMIT 1);";
	execQuery(sqlStatement, CallBackFunctions::createUser, &userList);
	return (userList.size() > 0) ? userList.front() : throw std::exception("No top tagged user");
}

Picture DatabaseAccess::getTopTaggedPicture()
{
	std::list<Picture> pictureList;
	std::string sqlStatement = "SELECT * FROM Pictures WHERE Id=(SELECT Picture_Id FROM Tags GROUP BY Picture_Id ORDER BY COUNT(Picture_Id) DESC LIMIT 1);";
	execQuery(sqlStatement, CallBackFunctions::createPicture, &pictureList);
	//Fill pictures with tags
	createCompletedPictures(pictureList);
	return (pictureList.size() > 0) ? pictureList.front() : throw std::exception("No top tagged picture");
}

std::list<Picture> DatabaseAccess::getTaggedPicturesOfUser(const User& user)
{
	std::list<Picture> pictureList;
	std::string sqlStatement = "SELECT * FROM Pictures WHERE Id IN (SELECT Picture_Id FROM Tags WHERE User_Id=" + std::to_string(user.getId()) + ");";
	execQuery(sqlStatement, CallBackFunctions::createPicture, &pictureList);
	//Fill pictures with tags
	createCompletedPictures(pictureList);
	return pictureList;
}
