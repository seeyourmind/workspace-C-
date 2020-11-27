#ifndef LEVENSHTEINAUTOMATA_H
#define LEVENSHTEINAUTOMATA_H
#include <list>
#include <forward_list>
#include <stack>
#include <vector>
// #include <hash_set>
#include <set>
#include <string>
#include <iostream>
#include <map>
#include <algorithm>

using namespace std;
// using namespace __gnu_cxx; // for hash_set
namespace LevenshteinAutomata
{
	const int ALPHABET_SIZE = 26;
	typedef struct TrieNode
	{
		struct TrieNode* children[ALPHABET_SIZE];
		// struct TrieNode* children;
		bool isEndWord;
		wchar_t key;
		wstring  value;
	}TrieNode;
	TrieNode *getNode(wchar_t key, wstring value);
	class Trie
	{
	public:
		Trie() {};
		virtual ~Trie() { free(rootNode); };
		TrieNode* rootNode;
		void insert(wstring key);
		bool search(wstring key);
	};

	class NFA
	{
	public:
		NFA() {};
		NFA(int size, int state, list<int> acceptStates);
		virtual ~NFA() { free(transTable); };
		static NFA* getLevenshteinAutomata(wstring str, int maxDist);
		void addTransition(int from, int to, wchar_t input);  // s -*-> a
		list<int> move(list<int> states, wchar_t inp);  // match process
		void show();

		int startState;
		list<int> acceptStates;
		vector<set<wchar_t> >* transTable;
		forward_list<wchar_t> inputs;
		int size;

		enum NondeterministicChar
		{
			ANY = '*',
			EPSILON = '#',
			DEAD = '\0'
		};
	/*private:
		bool NFA::ifEpsilonInThePath(int from, list<int> &resilt)*/
	};

	class DFA
	{
	public:
		DFA()
		{
			acceptStates = new list<int>();
			defaultTrans = new map<int, int>();
			transTable = new map<pair<int, wchar_t>, int>();
		};
		virtual ~DFA()
		{
			free(defaultTrans);
			free(acceptStates);
			free(transTable);
		};
		static DFA* getNFA2DFA(NFA *nfa);
		static list<int> epsilonClosure(NFA *nfa, list<int> states);
		void search(Trie *trie, int start, TrieNode* node, list<wstring> &output);
		void show();

		int start;
		list<int>* acceptStates;
		map<pair<int, wchar_t>, int> *transTable;
		map<int, int> *defaultTrans;
		list<wchar_t> uniqueChars;
	};
}
#endif // !LEVENSHTEINAUTOMATA_H