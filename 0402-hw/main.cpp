#include <iostream> 
#include <fstream>
#include <sstream>
#include <memory>
#include <vector>
#include <queue>
#include <chrono>
#include <iomanip> // std::setw
#include <cstdlib> // std::abs, std::system

#include <unistd.h>

#define UNTIL(x) while(not(x))

void print_mem()
{
	std::stringstream ss;
	ss << "procstat -r " << getpid();
	std::system(ss.str().c_str());
}

class node
{
public:
	node(int x, int y): x(x), y(y) {}

	template<typename T>
	void generate_child(int i)
	{
		static_assert(std::is_base_of<node, T>::value, "drived not derived from node");
		child.emplace_back(new T(x + i, y    ));
		child.emplace_back(new T(x - i, y    ));
		child.emplace_back(new T(x    , y + i));
		child.emplace_back(new T(x    , y - i));
		child.emplace_back(new T(x    , y    ));
		for (auto & n : child)
			n->parent = this;
	}

	static
	int& target_x() {static int x; return x;}
	static
	int& target_y() {static int y; return y;}
	
	node& set_target(int xx, int yy)
	{
		target_x() = xx;
		target_y() = yy;
		return *this;
	}
	bool is_target() {return x == target_x() && y == target_y();}
	bool has_child() {return !child.empty();}

	std::vector<node *> get_child()
	{
		std::vector<node *> x;
		for (auto &p : child)
			x.push_back(p.get());
		return x;
	}

	void backtrace(std::ostream &os) const
	{
		os << "[" << std::setw(3) << x << ", " << std::setw(3) << y << "]\n";
		if (parent)
			parent->backtrace(os);
	}	
	
protected:
	int x, y;
	node * parent = nullptr;
	std::vector<std::unique_ptr<node>> child;
};

class dfs_node : public node
{
public:
	dfs_node(int x, int y): node(x, y){}
	static
	bool visit(dfs_node * n, int depth, int limit)
	{
		if (depth > limit)
			return false;
		n->generate_child<dfs_node>(list().at(depth));
		for (auto &no: n->get_child())
		{
			auto c = static_cast<dfs_node *> (no);
			if (c->is_target())
			{
				print_mem();
						
				std::cout << "\n";
			    c->backtrace(std::cout);
				return true;
			}
			else if (visit(c, depth + 1, limit))
				return true;
		}
		return false;
	}

	static
	std::vector<int>& list()
	{
		static std::vector<int> list;
		return list;
	}
};


class A_star_node : public dfs_node
{
public:
	A_star_node(int x, int y): dfs_node(x, y){}
	int heuristic() {return abs(target_x() - x) / 9 + abs(target_y() - y) / 9;}

	void do_sort()
	{
		std::sort(child.begin(), child.end(), [](auto const &a, auto const &b) {
			return  static_cast<A_star_node *>(a.get())->heuristic() <
    				static_cast<A_star_node *>(b.get())->heuristic();
		});
	}

	static 
	bool visit(A_star_node * n, int now)
	{
    	if (now >= list().size())
			return false;
		n->generate_child<A_star_node>(list().at(now));
		n->do_sort();
	    for (auto &no: n->get_child())
		{
			auto c = static_cast<A_star_node *> (no);
			if (c->is_target())
			{
				print_mem();
						
			    c->backtrace(std::cout);
				return true;
			}
			else if (visit(c, now + 1))
				return true;
		}
		return false;
	}
};

std::ostream& operator << (std::ostream& os, node const & n)
{
	n.backtrace(os);
	return os;
}


int main (int argc, char *argv[])
{
	std::string line;
	while (std::getline(std::cin, line))
	{
		auto timer = [_S_ = std::chrono::steady_clock::now()]{return std::chrono::steady_clock::now() - _S_;};
		std::stringstream ss(line);
		std::string type;
		int targetx, targety;
		ss >> type >> targetx >> targety;

		if (type == "BFS")
		{
			node root{0, 0};
			std::queue<node *> unexplored;
			unexplored.push(&root);
			root.set_target(targetx, targety);
			std::cout << "Type BFS" << std::endl;

			int num;
			bool done{false};
			while (ss >> num && not done)
			{
				decltype(unexplored) next;
				UNTIL (unexplored.empty())
				{
					if (unexplored.front()->is_target())
					{
						print_mem();
								
						std::cout << *unexplored.front();
						done = true;
						break;
					}
					else
					{
						unexplored.front()->generate_child<node>(num);
						for (auto &c : unexplored.front()->get_child())
							next.push(c);
					}
					unexplored.pop();
				}
				next.swap(unexplored);
			}
		}
		else if (type == "IDS")
		{
			A_star_node root{0, 0};
			root.set_target(targetx, targety);

			std::cout << "Type IDS" << std::endl;
			std::vector<int> v;
			int num;
			while (ss >> num)
				v.push_back(num);
			dfs_node::list().swap(v);
			auto found = false;
			while (not found)
			{
				for (int i{0}; i < dfs_node::list().size(); i++)
				{
					std::cout << "[" << i << "] ";
					std::cout.flush();
					found = dfs_node::visit(&root, 0, i);
					if (found)
						break;
				}
			}			
		}
		else if (type == "A*")
		{
			A_star_node root{0, 0};
			root.set_target(targetx, targety);

			std::cout << "Type A*" << std::endl;
			std::vector<int> v;
			int num;
			while (ss >> num)
				v.push_back(num);
			A_star_node::list().swap(v);
			A_star_node::visit(&root, 0);
		}
		std::cout << "Elapsed time: "
				  << std::chrono::duration_cast<std::chrono::microseconds>(timer()).count() << " mms" << std::endl;
	}
}

