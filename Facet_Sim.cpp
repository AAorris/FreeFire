#include "stdafx.h"
#include <fstream>
#include <algorithm>
#include "Facet_Sim.h"
#include "Tool_Pos.h"
/*Bitset is an std implementation that allows for easy access
to bits one at a time(in contrast to bools, which are stored
in char(8 bit) containers), has a hash implementation, and
all sorts of other goodies.*/
#include <bitset>

/*=================================================
---------------------------------------------------
----------------------CHUNK------------------------
---------------------------------------------------
==================================================*/
using namespace CHUNK;
class Chunk
{
public:
	typedef	std::bitset<res>	t_item;
	typedef	std::bitset<total>	t_data;
	std::string			serialize() const;
	t_data*						getData();
	char						cell(const int& x, const int& y);
	void						setCell(const int& x, const int& y, const char& c);
	Chunk();
	~Chunk() = default;
	Chunk(const t_data& pdata);
	Chunk(const std::string& hex);
private:
	void operator=(const Chunk& c) = delete;
	Chunk(const Chunk& c) = delete;
	t_data* data;
};

class t_sim::Impl
{
public:
	typedef std::unordered_map<AA::Pos, unique_ptr<Chunk>> t_layer;
	typedef std::unordered_map<AA::Pos, unique_ptr<Chunk>> t_data;
	t_data data; //copy phobia?
	void add(const AA::Pos& p, const char& c);
	std::string getChunk(const AA::Pos& cellIndex);
	void putChunk(const std::string& s, const AA::Pos& ppos);
	void save(const std::string& fp);
	void load(const std::string& fp);
	Impl();
};


Chunk::Chunk()// : data {  } //<- std::unique_ptr<std::bitset<total>>(new std::bitset<total>())
{
	data = new t_data();
}

//#define HEX
#define HEX 1
#define ASCII 2
#define PACKED 3

#define SERIAL HEX

#if SERIAL == HEX
std::string Chunk::serialize() const
{
	std::stringstream ss;
	ss << std::hex << std::uppercase << data->to_ullong();
	return ss.str();
}

Chunk::Chunk(const std::string& sdata)
{
	std::string str{ sdata };
	str.erase(std::remove(begin(str), end(str), ' '), end(str));

	std::stringstream ss;
	ss << std::hex << str;
	unsigned long long n;
	ss >> n;
	data = new t_data(n);
}
#elif SERIAL == ASCII
std::string Chunk::serialize() const
{
	std::stringstream ss;
	std::bitset<8> b_char;
	for (int pos = 0; pos < total; pos += 4)
	{
		b_char.reset();
		for (int i = 0; i < res; i++)
			b_char.set(i, data->test(pos + i));
		char c = b_char.to_ullong();
		c += 32;
		ss << c;
	}
	return ss.str();
}
Chunk::Chunk(const std::string& sdata)
{
	int res = 0;
	for (int i = 0; i < sdata.length(); i++)
	{
		res += (char)sdata[i] << 8*i;
	}
	data = new t_data(res);
}
#elif SERIAL == PACKED
std::string Chunk::serialize() const
{
	std::stringstream ss;
	std::bitset<8> b_char;
	for (int pos = 0; pos < total; pos += 8)
	{
		b_char.reset();
		for (int i = 0; i < 8; i++)
			b_char.set(i, data->test(pos + i));
		char c = b_char.to_ullong();
		c += 32;
		if (c>126)
			throw 0;
		ss << c;
	}
	return ss.str();
}
Chunk::Chunk(const std::string& sdata)
{
	std::bitset<64> res;
	for (int i = 0; i < sdata.length(); i++)
	{
		auto bits = std::bitset<8>(sdata[i]);
		for (int bit = 0; bit < 8; bit++)
			res.set(i * 8 + bit, bits.test(bit));
	}
	data = new t_data(res);
}
#endif


/**Go ahead and use only the first parameter for 1d reference. */
char Chunk::cell(const int& x, const int& y)
{
	t_item set = t_item();
	for (int i = 3; i >= 0; i--)
		set.set(i, data->test(total - 1 - (i + x*res + y*width*res)));
	return (char)set.to_ulong();
}
void Chunk::setCell(const int& x, const int& y, const char& c)
{
	int n = c;
	if (c >= 16)
		n -= 55;
	t_item in = t_item(n);
	for (int bit = 3; bit >= 0; bit--)
		data->set(total - 1 - (bit + x*res + y*width*res), in.test(3-bit));
}

/*=================================================
---------------------------------------------------
----------------------SIM-IMPL---------------------
---------------------------------------------------
==================================================*/

t_sim::Impl::Impl() : data{ t_layer() }
{
}

void t_sim::Impl::add(const AA::Pos& p, const char& c)
{
	auto p_chunk = AA::Pos(p.x() / CHUNK::width, p.y() / CHUNK::height);
	auto it = data.find(p_chunk);
	if (it == end(data))
		data[p_chunk] = wrap(new Chunk());
	data[p_chunk]->setCell(abs(p.x()) % CHUNK::width, abs(p.y()) % CHUNK::height, c);
	
}

std::string t_sim::Impl::getChunk(const AA::Pos& cellIndex)
{
	auto it = data.find(cellIndex);
	if (it == end(data))
		return "";
	else
		return data.at(cellIndex)->serialize();
}

void t_sim::Impl::putChunk(const std::string& s, const AA::Pos& ppos)
{
	data[ppos] = wrap(new Chunk(s));
}

void t_sim::Impl::save(const std::string& fp)
{
	auto fout = std::ofstream{ fp };
	for (auto& chunk : data)
	{
		auto cpos = chunk.first;
		auto& cdata = chunk.second;
		fout << cpos << " ";
		fout << cdata->serialize() << "\n";
	}
	fout.close();
}


void t_sim::Impl::load(const std::string& fp)
{
	auto fin = std::ifstream{ fp };
	AA::Pos cpos;
	std::string cstr;
	while (fin >> cpos)
	{
		fin >> cstr;
		data[cpos] = wrap(new Chunk{ cstr });
	}
	fin.close();
}

namespace AA {
	template<> struct serialize<std::pair<AA::Pos, Chunk>> {
		std::string operator()(const std::pair<AA::Pos, Chunk>& p) const {
			std::stringstream res;
			res << static_cast<char>(p.first.x()) << static_cast<char>(p.first.y());
			res << AA::serialize<Chunk>()(p.second);
			return res.str();
		}
	};
}

/*=================================================
---------------------------------------------------
----------------------SIM-DEF----------------------
---------------------------------------------------
These defs really need to be put in one place...
==================================================*/

Facet_Sim::Facet_Sim() : p{ new Impl() }
{
}

Facet_Sim::~Facet_Sim()
{
}

void t_sim::loadState(const std::string& fp)
{
	p->load(fp);
}

void t_sim::saveState(const std::string& fp)
{
	p->save(fp);
}

void t_sim::put(const AA::Pos& ppos, const char& pid)
{
	p->add(ppos, pid);
}

std::string t_sim::getChunk(const AA::Pos& cellIndex)
{
	return p->getChunk(cellIndex);
}

void t_sim::putChunk(const std::string& s, const AA::Pos& ppos)
{
	p->putChunk(s,ppos);
}