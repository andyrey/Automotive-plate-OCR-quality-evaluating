/*A.Zakharoff
This program for evaluation OCR methods for APNR.It takes folder name and pick up all txt files
into a list,
which are pairwise- one for OCR-read from the picture, another corresponding ground truth.
Compare all pairs in all files and estimate edit distance between two strings.
*/
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <iostream>
#include "dirent.h"
//#define DEBUG
using namespace std;
struct aligned_strings
{
	string str_gt;
	string str_ocr_found;
	string str_gt_aligned;
	string str_ocr_aligned;
	//int distance;
};
template<typename T>
T min_3(T& a, T&  b, T& c)
{
	T min3 = (a <= b && a < c) ? a : ((b < c) ? b : c);
	return min3;
}
void outputAlignment(const int& i, const int& j, const vector<vector<float>>& D,
	const string& gt_str, const string& read_str, string& gt_str_aligned, string& read_str_aligned)
{
	if (i == 0 && j == 0)
		return;

	//string gt_str_aligned,read_str_aligned;
	if (i > 0 && D[i][j] == (D[i - 1][j] + 1))//downmove
	{
		outputAlignment(i - 1, j, D, gt_str, read_str, gt_str_aligned, read_str_aligned);
		std::cout << gt_str[i] << "  " << "-" << std::endl;
		gt_str_aligned += gt_str[i];
		read_str_aligned += "-";
	}
	else
	{
		if (j > 0 && D[i][j] == (D[i][j - 1] + 1))//to right
		{
			outputAlignment(i, j - 1, D, gt_str, read_str, gt_str_aligned, read_str_aligned);
			std::cout << "-" << "  " << read_str[j] << std::endl;
			gt_str_aligned += "-";
			read_str_aligned += read_str[j];//it was i 
		}
		else
		{
			outputAlignment(i - 1, j - 1, D, gt_str, read_str, gt_str_aligned, read_str_aligned);//diagon to right-down
			std::cout << gt_str[i] << "  " << read_str[j] << std::endl;
			gt_str_aligned += gt_str[i];
			read_str_aligned += read_str[j];
		}

	}

}
void  removeSpaces(string& str)
{
	str.erase(remove(str.begin(), str.end(), ' '), str.end());
	//return str;
}
float edit_distance(const string &gt_str, const string &read_str, string& gt_str_aligned, string& evaluated_str_aligned, bool shouldbealigned = false)
{
	string s1 = " " + gt_str;//for zero row and column
	string s2 = " " + read_str;
	
	//convert all into single upper case:
	for (auto & c : s1) c = toupper(c);
	for (auto & c : s2) c = toupper(c);

	int length1 = s1.length();
	int length2 = s2.length();
	vector<vector<float>> matrix;
	matrix.resize(length1, std::vector<float>(length2, 0));//n_rows x m_columns
	float insertion, deletion, match, mismatch,mismatch2;
	int i, j;
	for (i = 0; i < length1; ++i)//row
		matrix[i][0] = float(i);
	for (j = 0; j < length2; ++j)//col
		matrix[0][j] =float(j);

	for (i = 1; i < length1; ++i)//row
	{
		for (j = 1; j < length2; ++j)//col
		{
			insertion = matrix[i][j - 1] + 1.f;// 1;
			deletion = matrix[i - 1][j] + 1.f;
			match = matrix[i - 1][j - 1];
			mismatch = matrix[i - 1][j - 1] + 1.f;
			mismatch2 = matrix[i - 1][j - 1] + 0.5f;
			if (s1[i] == s2[j])
				matrix[i][j] = min_3<float>(insertion, deletion, match);
			else
			{
				int a = 0;
				if (s2[j] == '?')
					matrix[i][j] = min_3<float>(insertion, deletion, mismatch2);
				else
					matrix[i][j] = min_3<float>(insertion, deletion, mismatch);
			}
		}
	}
	///print aligned strings в задании не надо
	//if(shouldbealigned)
	outputAlignment(length1 - 1, length2 - 1, matrix, s1, s2, gt_str_aligned, evaluated_str_aligned);

	return matrix[length1 - 1][length2 - 1];
}

struct compare {
	bool operator()(const std::string& first, const std::string& second)
	{
		return first.size() < second.size();
	}
};

bool collect_filenames_to_list(const std::string& dir_name, std::list<std::string>& list_of_entities1, std::list<std::string>& list_of_entities2, const string& fn_diff)//, int doc_type
{//takes all txt files in the folder and insert their names to the list
	DIR *dir;//getting all files from given dir
	struct dirent *ent;
	const char *cchar_dir = dir_name.c_str();
	if ((dir = opendir(cchar_dir)) != NULL)
	{
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL)
		{
			printf("%s\n", ent->d_name);
			std::string each_filename(ent->d_name);
			std::string const file_ext = each_filename.substr(each_filename.find_last_of(".") + 1);
			if (file_ext == "txt")
			{
				if (each_filename.find(fn_diff) != std::string::npos)
					list_of_entities2.push_back(dir_name + each_filename);
				else
					list_of_entities1.push_back(dir_name + each_filename);

			}
		}
		closedir(dir);
	}
	else
	{
		/* could not open directory */
		std::cout << " failed to open dir " << dir_name << " when looking for ";
	}
	compare c;
	list_of_entities1.sort(c);
	list_of_entities2.sort(c);
	return (list_of_entities1.size()== list_of_entities2.size()); //returns number of entities-files
}
bool compare_filenames(const string& fn1, const string& fn2, const string& fn_diff)
{//a1image_.txt a1image_gt.txt - difference should be only as "gt"
	int min_length;// = max(fn1.length, fn2.length);
	const string* another_string;
	string composite_string;
	if (fn1.length() < fn2.length())
	{
		another_string = &fn2;
		composite_string = fn1;
		//min_length = fn1.length;
	}
	else
	{
		another_string = &fn1;
		composite_string = fn2;
		//min_length = fn2.length;
	}
	min_length = composite_string.length();
	composite_string.insert(min_length - 4, fn_diff);
	return composite_string.compare(*another_string);
}

void formPairs(const string& gt_filename, const string& ocr_filename, std::multimap<float, aligned_strings>& mmInt_Al_str)//, int& gt_num, int& detections_num
{//takes inputs 2 files= gt and ocr, and fills number of corresponding objects there
	
	ifstream txt_ocr, txt_gt;
	//gt_num = 0; detections_num = 0;
	int local_plates_found = 0, local_plates_gt = 0;
	float distance;
	//txt_ocr.open(ocr_filename);
	txt_gt.open(gt_filename);
	string  apnr_string_ocr, apnr_string_gt;//
	list<string> list_apnr_ocr;
	string string_gt_alnd, string_ocr_alnd;
	//std::multimap<int, aligned_strings> mmInt_Al_str;
	//report << ocr_filename << endl;
	
	while (std::getline(txt_gt, apnr_string_gt))
	{ //go along gt file strings
		removeSpaces(apnr_string_gt);//some files can contain empty line
		if (apnr_string_gt.empty())
			continue;

		txt_ocr.open(ocr_filename);
		while (std::getline(txt_ocr, apnr_string_ocr))
		{//fill list of apnr string by ocr program
			removeSpaces(apnr_string_ocr);
			if (apnr_string_ocr.empty())
				continue;
			std::pair<float, aligned_strings> mypair;
			distance = edit_distance(apnr_string_gt, apnr_string_ocr, string_gt_alnd, string_ocr_alnd, true);
			//if (distance == -1)//empty
			//	continue;
			aligned_strings as;
			//as.distance = distance;
			as.str_gt = apnr_string_gt;
			as.str_ocr_found = apnr_string_ocr;
			as.str_gt_aligned = string_gt_alnd;
			as.str_ocr_aligned = string_ocr_alnd;
			mypair.first = distance;
			mypair.second = as;
			mmInt_Al_str.insert(mypair);
			apnr_string_ocr.clear();
			apnr_string_ocr.shrink_to_fit();
			string_gt_alnd.clear();
			string_ocr_alnd.clear();
			string_gt_alnd.shrink_to_fit();
			string_ocr_alnd.shrink_to_fit();
		}
		txt_ocr.close();
		apnr_string_gt.clear();
		apnr_string_gt.shrink_to_fit();
	}
	txt_gt.close();
}

void filterPairs(std::multimap<float, aligned_strings>& mmInt_Al_str,set<string>& set_notfound,  int& gt_num, int& detections_num)//,const int gt_num, const int detections_num
{
	//I create these sets to get rid off duplicating in above pairs
	set<string> set_string_gt, set_string_ocr;
	//let us get rid off redundand pairs in multimap
	for (auto iter_multmap = mmInt_Al_str.begin(); iter_multmap != mmInt_Al_str.end();)
	{
		auto i_gt = set_string_gt.insert(iter_multmap->second.str_gt);
		auto i_ocr = set_string_ocr.insert(iter_multmap->second.str_ocr_found);
		if (!i_gt.second || !i_ocr.second)//bcz ocr or g.t. unique strings have already saved in the set
			iter_multmap = mmInt_Al_str.erase(iter_multmap);
		else
			++iter_multmap;
	}
	set<string> set_found;//set with all read plates
	for (auto iter_multmap = mmInt_Al_str.begin(); iter_multmap != mmInt_Al_str.end(); ++iter_multmap)
		set_found.insert(iter_multmap->second.str_gt);
	//let find all strings which are in g.t. but not in read by ocr:
	//not_found= set_string_gt-set_found
	std::set_difference(set_string_gt.begin(), set_string_gt.end(), set_found.begin(),
		set_found.end(), std::inserter(set_notfound, set_notfound.end()));
	gt_num = set_string_gt.size();
	detections_num = set_found.size();
}
string  pop_next( list<string>& ls)
{
	string s = "";
	if (!ls.empty())
	{
		s=ls.front();
		ls.pop_front();
		
	}
	return s;
}

void main(int argc, char *argv[])
{
#ifndef DEBUG
	if (argc < 2) {
		std::cout << "*.exe Path/Folder_with_results" << std::endl;
		return;
	}
#else
	argv[1] = "D:\\vs_projects\\MY_CURRENT_WORK\\EvaluateOCR\\eval_ocr\\x64\\Release\\Compare\\";
#endif
	string str_diff = "gt";
	ofstream file_report;
	file_report.open("report.txt");
	int total_plates_gt = 0, total_plates_ocr=0,  plates_read_with_error = 0, total_symbols=0;
	float overall_symbol_error = 0.0f;
	string folder_path = argv[1];
	std::list<string> file_list_ocr,file_list_gt;
	if (!collect_filenames_to_list(folder_path, file_list_ocr, file_list_gt, str_diff))
		std::cout << "not all ocr and gt files paired!!!" << std::endl << std::endl;
	while (!file_list_ocr.empty())
	{
		string filename1 = pop_next(file_list_ocr);
		string filename2 = pop_next(file_list_gt);
		if (compare_filenames(filename1, filename2, str_diff))
		{
			std::cout << filename1 << " doesn't has it's pair with " + str_diff << endl;
			continue;
		}
		
		//compare_2files(filename1, filename2,
		//	overall_error, plates_read_with_error,
		//	total_plates_gt, total_plates_ocr, file_report);
		std::multimap<float, aligned_strings> mmInt_Al_struct;
		int gt_num,  detections_num;
		file_report << filename1 << " versus " << filename2 << endl;
		formPairs(filename2,filename1,  mmInt_Al_struct);
		set<string> not_found_set;
		filterPairs(mmInt_Al_struct, not_found_set, gt_num, detections_num);///добавить учет дистанции, ошибок в платах
		for (auto it = mmInt_Al_struct.begin(); it != mmInt_Al_struct.end(); ++it)
		{
			file_report << "g.t. " << it->second.str_gt << " read as " << it->second.str_ocr_found << endl;
			file_report << "g.t. aligned:  " << it->second.str_gt_aligned << endl;
			file_report << "ocr   aligned: " << it->second.str_ocr_aligned << endl;
			file_report << "error: " << it->first << endl;
			overall_symbol_error += it->first;
			total_symbols += it->second.str_gt.length();
			if(it->first)//ненулевая ошибка
			++plates_read_with_error;
		}
		for (auto iset : not_found_set)//these gt strings havn't been found
			file_report << "not found " << iset << endl;
		total_plates_gt += gt_num;
		total_plates_ocr += detections_num;
		mmInt_Al_struct.clear();
		file_report << endl;
		
	}
	
	file_report << "SUMMARY:" << endl;
	file_report << "plates detected " << total_plates_ocr << " from " << total_plates_gt << endl;
	file_report << "overall  errors in OCR " << overall_symbol_error <<" of "<< total_symbols<<
		" symbols - accuracy "<< (100.f*((float)total_symbols-overall_symbol_error)/total_symbols)<<"%"<<endl;
	file_report << "plates read with at least one error " << plates_read_with_error;
	file_report << " from all " << total_plates_ocr << " detected " << endl;
	file_report << " accuracy above detected & read:  " << 100.f*(total_plates_ocr - plates_read_with_error) / total_plates_ocr << "%"<<endl;
	file_report << " accuracy above all ground truth: " << 100.f*(total_plates_ocr - plates_read_with_error) / total_plates_gt << "%" << endl;
	file_report.close();
	return;
}


