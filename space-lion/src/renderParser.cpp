#include "renderParser.h"

renderParser::renderParser()
{
}

renderParser::~renderParser()
{
}

bool renderParser::parseMaterial(const char* const materialPath, materialInfo& inOutMtlInfo)
{
	std::string buffer;
	std::string tempStr;
	std::string::iterator iter1;
	std::string::iterator iter2;

	std::ifstream file;
	file.open(materialPath, std::ifstream::in);

	if( file.is_open() )
	{
		file.seekg(0, std::ifstream::beg);

		std::getline(file,buffer,'\n');
		inOutMtlInfo.id = atoi(buffer.c_str());

		while(!file.eof())
		{
			std::getline(file,buffer,'\n');
			
			iter2 = buffer.begin();
			iter1 = buffer.begin();
			iter1++;iter1++;
			tempStr.assign(iter2,iter1);

			if(tempStr == "td")
			{
				iter2 = (iter1 + 1);
				iter1 = buffer.end();
				tempStr.assign(iter2,iter1);
				inOutMtlInfo.diff_path = new char[tempStr.length()];
				strcpy((inOutMtlInfo.diff_path),tempStr.c_str());
			}
			else if(tempStr == "ts")
			{
				iter2 = (iter1 + 1);
				iter1 = buffer.end();
				tempStr.assign(iter2,iter1);
				inOutMtlInfo.spec_path = new char[tempStr.length()];
				strcpy((inOutMtlInfo.spec_path),tempStr.c_str());
			}
			else if(tempStr == "tn")
			{
				iter2 = (iter1 + 1);
				iter1 = buffer.end();
				tempStr.assign(iter2,iter1);
				inOutMtlInfo.normal_path = new char[tempStr.length()];
				strcpy((inOutMtlInfo.normal_path),tempStr.c_str());
			}
			else
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}
	return true;
}