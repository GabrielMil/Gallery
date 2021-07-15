#pragma once
#include <iostream>
#include <list>
#include "Album.h"
#include "User.h"

class CallBackFunctions
{
public:
	//This function works when SQL returs single number
	static int getSingleNum(void* pNum, int argc, char** argv, char** azColName)
	{
		(*((int*)pNum)) = atoi(argv[0]);  // Get the first and only argument
		return 0;
	}

	//This function creates an album from given SQL response
	static int createAlbum(void* pAlbumList, int argc, char** argv, char** azColName)
	{
		Album album;
		std::list<Picture> pictures;
		for (int i = 0; i < argc; i++)
		{
			if (std::string(azColName[i]) == "Name")
			{
				album.setName(argv[i]);
			}
			else if (std::string(azColName[i]) == "Creation_Date")
			{
				album.setCreationDate(argv[i]);
			}
			else if (std::string(azColName[i]) == "User_Id")
			{
				album.setOwner(atoi(argv[i]));
			}
		}
		((std::list<Album>*)pAlbumList)->push_back(album);
		return 0;
	}

	//This function creates an picture from given SQL response
	static int createPicture(void* pPictureList, int argc, char** argv, char** azColName)
	{
		int id = 0, albumID = 0;
		std::string name, path, creationDate;
		//Create the base picture
		for (int i = 0; i < argc; i++)
		{
			if (std::string(azColName[i]) == "Name")
			{
				name = argv[i];
			}
			else if (std::string(azColName[i]) == "Creation_Date")
			{
				creationDate = argv[i];
			}
			else if (std::string(azColName[i]) == "Location")
			{
				path = argv[i];
			}
			else if (std::string(azColName[i]) == "Album_Id")
			{
				albumID = atoi(argv[i]);
			}
			else if (std::string(azColName[i]) == "Id")
			{
				id = atoi(argv[i]);
			}
		}
		((std::list<Picture>*)pPictureList)->push_back(Picture(id, name, path, creationDate));
		return 0;
	}

	//This function creates an user from given SQL response
	static int createUser(void* pUserList, int argc, char** argv, char** azColName)
	{
		int id = 0;
		std::string name;
		for (int i = 0; i < argc; i++)
		{
			if (std::string(azColName[i]) == "Id")
			{
				id = atoi(argv[i]);
			}
			else if (std::string(azColName[i]) == "Name")
			{
				name = argv[i];
			}
		}
		((std::list<User>*)pUserList)->push_back(User(id, name));
		return 0;
	}

	//This function counts the amount of the rows
	static int countRows(void* pCount, int argc, char** argv, char** azColName)
	{
		(*((int*)pCount))++;
		return 0;
	}

	//This function gets the number into the list
	static int getNumbers(void* pNumbers, int argc, char** argv, char** azColName)
	{
		//Return only 1 number but we put it in list
		((std::list<int>*)pNumbers)->push_back(atoi(argv[0]));
		return 0;
	}
};

//int name(void* fourthParam, int argc, char** argv, char** azColName)
