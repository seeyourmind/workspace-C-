#include "LevenshteinAutomata.h"

#include <ctime>
#include <fstream>

namespace LevenshteinAutomata
{
	/*******************************************************/
	/*                         NFA                         */
	/*******************************************************/
	NFA::NFA(int _size, int state, list<int> _acceptStates)
	{
		size = _size;
		startState = state;
		acceptStates = _acceptStates;
		transTable = new vector<set<wchar_t> >();
		// intialize the transition process of NFA using '\0'
		set<wchar_t> sc; sc.insert('\0');
		for (size_t i = 0; i < size*size; ++i)
			transTable->emplace_back(sc);
		printf("initalize NFA complete!\n");
		sc.clear();
	}

	NFA* NFA::getLevenshteinAutomata(wstring str, int maxDist)
	{
		int width = str.length() + 1;
		int height = maxDist + 1;
		int size = width*height;

		list<int> acceptStates;
		// initialize acceptStates
		for (size_t i = 1; i <= height; ++i)
			acceptStates.emplace_back(i*width - 1);
		NFA* nfa = new NFA(size, 0, acceptStates);
		// add transition process
		for (size_t e = 0; e < height; ++e)
		{
			for (size_t i = 0; i < width - 1; ++i)
			{
				// correct character
				nfa->addTransition(e*width + i, e*width + (i + 1), str[i]);
				if (e < maxDist)
				{
					// deletion
					nfa->addTransition(e*width + i, (e + 1)*width + (i + 1), (wchar_t)NondeterministicChar::EPSILON);
					// insertion
					nfa->addTransition(e*width + i, (e + 1)*width + i, (wchar_t)NondeterministicChar::ANY);
					// substitution
					nfa->addTransition(e*width + i, (e + 1)*width + (i + 1), (wchar_t)NondeterministicChar::ANY);
				}
			}
			int eup = e + 1;
			if (eup < height)
				nfa->addTransition(eup*width - 1, (eup + 1)*width - 1, NondeterministicChar::ANY);
		}
		return nfa;
	}

	void NFA::addTransition(int from, int to, wchar_t input)
	{
		set<wchar_t> s = transTable->at((from*size) + to);
		// printf("Construct: from %d to %d input %c  %d\n", from, to, input, s.size());
		if (*s.begin() == (wchar_t)NFA::NondeterministicChar::DEAD)
			transTable->at((from*size) + to).clear();
		transTable->at((from*size) + to).insert(input);
		// if (input != (char)NFA::NondeterministicChar::EPSILON)
		inputs.emplace_front(input);
		inputs.sort();
		inputs.unique();
		s.clear();
	}

	list<int> NFA::move(list<int> states, wchar_t inp)
	{
		/*printf("[input %c]states: ", inp);
		for (int i : states)
			printf("%d %", i);
		printf("\n");*/
		list<int> result;
		bool needNormalLetter = false;  // need deterministic char in transition path
		bool findNormalLetter = false;  // find deterministic char in transition path
		if (inp != (wchar_t)NFA::NondeterministicChar::EPSILON &&
			inp != (wchar_t)NFA::NondeterministicChar::ANY)
			needNormalLetter = true;
		for (list<int>::iterator it = states.begin(); it != states.end(); ++it)
		{
			for (size_t j = 0; j < size; ++j)
			{
				set<wchar_t> accept_cs = transTable->at((*it * size) + j);
				if (accept_cs.find(inp) != accept_cs.end())
				{
					if (needNormalLetter && accept_cs.find(inp) != accept_cs.end()) findNormalLetter = true;
					result.emplace_back(j);
				}
			}
		}
		if (needNormalLetter && !findNormalLetter) result.clear();
		result.sort();
		result.unique();
		// sort(result.begin(), result.end());
		// result.erase(unique(result.begin(), result.end()));
		/*printf("result: ");
		for (int i : result)
			printf("%d %", i);
		printf("\n");*/
		return result;
	}

	void NFA::show()
	{
		printf("This NFA has %d states: 0 - %d\n", size, size - 1);
		printf("The initial state is %d\n", startState);
		printf("The final state is %d\n", acceptStates);
		printf("The transition table size is %d\n", transTable->size());
		for (int from = 0; from < size; ++from)
		{
			for (size_t to = 0; to < size; ++to)
			{
				set<wchar_t> in = transTable->at((from * size) + to);
				// printf("Show: from %d to %d input %d\n", from, to, in.size());
				if (in.find((wchar_t)NFA::NondeterministicChar::DEAD)==in.end())
				{
					if (in.find((wchar_t)NFA::NondeterministicChar::EPSILON) != in.end())
						cout << "Transition from " << from << " to " << to << " on input " << (wchar_t)NFA::NondeterministicChar::EPSILON << " [delete]" << endl;
					if (in.find((wchar_t)NFA::NondeterministicChar::ANY)!=in.end())
						cout << "Transition from " << from << " to " << to << " on input " << (wchar_t)NFA::NondeterministicChar::ANY << " [insert or subsutite]" << endl;
					if (in.find((wchar_t)NFA::NondeterministicChar::EPSILON) == in.end() &&
						in.find((wchar_t)NFA::NondeterministicChar::ANY) == in.end())
						cout << "Transition from " << from << " to " << to << " on input " << *in.begin() << " [insert or subsutite]" << endl;
				}
			}
		}
		printf("\n");
	}

	/*******************************************************/
	/*                         DFA                         */
	/*******************************************************/
	/** Illumination of NFA2DFA
	* First, null-string closure [e-closure(s0)] is the unique state of DFA's states, and no marked.
	* while (no marked state:T in DFA's states) {
	*	mark T;
	*	for each (char in input) {
	*		U = e-closure(move(T, a));
	*		if (U no in DFAstates)
	*			DFAstates.push(U), and no marked;
	*		D_tran[T, a] = U;
	*	}
	* }*/
	DFA* DFA::getNFA2DFA(NFA *nfa)
	{
		int num = 0;
		DFA* dfa = new DFA();
		/*printf("NFA's finalStates: ");
		for (int finals : nfa->acceptStates)
		printf("%d ", finals);
		printf("\n");
		printf("NFA's inputs: ");
		for (char inp : nfa->inputs)
		printf("%c ", inp);
		printf("\n");
		return dfa;*/

		list<list<int>> markedStates;  // store deterministic states
		list<list<int>> unmarkedStates;  // store nondeterministic states
		map<list<int>, int> dfaStateNum;  // new states of DFA, which is composed of state clusters in the original NFA 

		list<int> nfaInitial;
		nfaInitial.emplace_back(nfa->startState);

		list<int> first = epsilonClosure(nfa, nfaInitial);  // get epsilon's kleene closure
		/*printf("first e-closure: ");
		for (int i : first)
			printf("%d ", i);
		printf("\n");*/
		unmarkedStates.emplace_back(first);

		//the initial dfa state
		int dfaInitial = num++;
		dfa->start = dfaInitial;
		dfaStateNum[first] = dfaInitial;

		while (unmarkedStates.size() != 0)
		{
			// mark T
			list<int> aState = unmarkedStates.front();
			unmarkedStates.pop_front();
			markedStates.emplace_back(aState);

			// confirm DFA's acceptStates
			for (list<int>::iterator it = aState.begin(); it != aState.end(); ++it)
			{
				for (list<int>::iterator it2 = nfa->acceptStates.begin(); it2 != nfa->acceptStates.end(); ++it2)
				{
					if (*it == *it2)
						dfa->acceptStates->emplace_back(dfaStateNum[aState]);
				}
			}
			// mave(T,a)
			for (wchar_t c : nfa->inputs)
			{
				if (c == (wchar_t)NFA::NondeterministicChar::EPSILON) continue;
				list<int> next = epsilonClosure(nfa, nfa->move(aState, c));
				/*printf("[(");
				for (int i : aState)
					printf("%d ", i);
				printf(") input %c] next e-closure: ", c);
				for (int i : next)
					printf("%d ", i);
				printf("\n*******************************************************\n");*/
				//continue;
				
				if (next.empty()) continue;
				// record new state
				if (find(unmarkedStates.begin(), unmarkedStates.end(), next) == unmarkedStates.end() &&
					find(markedStates.begin(), markedStates.end(), next) == markedStates.end())
				{
					unmarkedStates.emplace_back(next);
					dfaStateNum.emplace(next, num++);
				}
				// record transPath
				dfa->uniqueChars.emplace_back(c);
				pair<int, wchar_t> transition = make_pair(dfaStateNum[aState], c);
				dfa->transTable->emplace(pair<pair<int, wchar_t>, int>(transition, dfaStateNum[next]));
			}
		}

		dfa->acceptStates->sort();
		dfa->acceptStates->unique();
		dfa->uniqueChars.sort();
		dfa->uniqueChars.unique();

		/*sort(dfa->acceptStates->begin(), dfa->acceptStates->end());
		dfa->acceptStates->erase(unique(dfa->acceptStates->begin(), dfa->acceptStates->end()));
		sort(dfa->uniqueChars.begin(), dfa->uniqueChars.end());
		dfa->uniqueChars.erase(unique(dfa->uniqueChars.begin(), dfa->uniqueChars.end()));*/

		return dfa;
	}

	list<int> DFA::epsilonClosure(NFA * nfa, list<int> states)
	{
		if (states.empty()) return states;

		stack<int> uncheckedStack;

		//Push all states onto stack
		for (list<int>::iterator it = states.begin(); it != states.end(); ++it)
			uncheckedStack.push(*it);

		list<int> epsilonClosure = states;

		while (!uncheckedStack.empty())
		{
			int t = uncheckedStack.top();
			uncheckedStack.pop();
			int startIndex = 0;
			for (size_t i = 0; i < nfa->size; ++i)
			{
				set<wchar_t> input = nfa->transTable->at((t* nfa->size) + i);
				if (input.find((wchar_t)LevenshteinAutomata::NFA::NondeterministicChar::EPSILON)!=input.end())
				{
					if (find(epsilonClosure.begin(), epsilonClosure.end(), startIndex) == epsilonClosure.end())
					{
						epsilonClosure.emplace_back(startIndex);
						uncheckedStack.push(startIndex);
					}
				}
				startIndex++;
			}
		}
		return epsilonClosure;
	}

	void DFA::show()
	{
		printf("The initial state is %d\n", start);
		printf("The final state is %d\n", acceptStates->size());
		for (map<pair<int, wchar_t>, int>::iterator it = transTable->begin(); it != transTable->end(); ++it)
			cout << "Transition from " << it->first.first << " to " << it->second << " on input " << it->first.second << endl;
		printf("Accept States include: ");
		for (list<int>::iterator it = acceptStates->begin(); it != acceptStates->end(); ++it)
			printf("%d ", *it);
		printf("\n");
	}

	void DFA::search(Trie *trie, int start, TrieNode* node, list<wstring> &output)
	{
		// printf("start is [ %d ]\n", start);
		// if start is a acceptState, add start into output
		if (find(acceptStates->begin(), acceptStates->end(), start) != acceptStates->end() && node->isEndWord)
		{
			output.emplace_back(node->value);
			// output.insert(node->value);
			// cout << "get match string is " << node->value << endl;
		}
			
		// record accept input from DFA
		list<char> inputs;
		for (list<wchar_t>::iterator it = uniqueChars.begin(); it != uniqueChars.end(); ++it)
		{
			pair<int, wchar_t> pair = make_pair(start, *it);
			auto it2 = transTable->find(pair);
			if (it2 != transTable->end())
			{
				inputs.emplace_back(*it);
				for (size_t i = 0; i < ALPHABET_SIZE; ++i)
				{
					if (node->children[i] == NULL) continue;
					if (node->children[i]->key == *it || *it == NFA::NondeterministicChar::ANY)
					{
						// printf("node char is %c, dfa char is %c\n", node->children[i]->key, *it);
						search(trie, transTable->at(pair), node->children[i], output);
					}
						
				}
			}
		}
		/*auto it = defaultTrans->find(start);
		if (it != defaultTrans->end())
		{
			for (size_t i = 0; i < ALPHABET_SIZE; ++i)
			{
				bool found = false;
				if (node->children[i] == NULL)continue;
				for (list<char>::iterator it = inputs.begin(); it != inputs.end(); ++it)
					if (*it == node->children[i]->key || *it == NFA::NondeterministicChar::ANY) found = true;
				if (!found) search(trie, defaultTrans->at(start), node->children[i], output);
			}
		}*/
		output.sort();
		output.unique();
	}

	/*******************************************************/
	/*                        Trie                         */
	/*******************************************************/
	// get a initial node
	TrieNode* getNode(wchar_t key, wstring value)
	{
		TrieNode* pNode = new TrieNode;
		pNode->key = key;
		pNode->value = value;
		pNode->isEndWord = false;

		for (size_t i = 0; i < ALPHABET_SIZE; ++i)
			pNode->children[i] = NULL;

		return pNode;
	}

	void Trie::insert(wstring key)
	{
		struct TrieNode* pCrawl = rootNode;
		wstring k = L"";
		for (size_t i = 0; i < key.length(); ++i)
		{
			int index = key[i] - 'a';
			k += key[i];
			if (!pCrawl->children[index])
			{
				pCrawl->children[index] = getNode(key[i], k);
			}
			pCrawl = pCrawl->children[index];
		}
		// mark last node as leaf
		pCrawl->isEndWord = true;
		// cout << "key is " << key << ", k is " << k << endl;
	}

	bool Trie::search(wstring key)
	{
		struct TrieNode* pCrawl = rootNode;
		for (size_t i = 0; i < key.length(); ++i)
		{
			int index = key[i] - 'a';
			if (!pCrawl->children[index])
				return false;
			pCrawl = pCrawl->children[index];
		}
		return (pCrawl != NULL&&pCrawl->isEndWord);
	}
}

//using namespace LevenshteinAutomata;
/*int main_file(int argc, char **argv)
{
	ifstream file;
	LevenshteinAutomata::Trie trie;
	clock_t timer;
	double duration = 0;
	double totalTime = 0;
	int maxDist = 2;
	LevenshteinAutomata::NFA* nfa;
	LevenshteinAutomata::DFA* dfa;
	char line[256];

	cout << "Constructing Candidate Keys Trie..." << endl;
	timer = clock();
	trie.rootNode = LevenshteinAutomata::getNode(' ', "");
	// load in all resources
	// vector<string> testList = { "fxod", "fd", "fxd", "d", "od", "sxfxod", "abood" };
	file.open("E:\\LevenshteinsAutomata-master\\wordList.txt");
	//for (string line : testList)
	while (file.good())
	{
		file.getline(line, 256);
		trie.insert(line);
	}
	file.close();
	duration = (clock() - timer) / (double)CLOCKS_PER_SEC;
	cout << "Constructing Trie Complete! Time: " << duration << " Seconds" << endl;
	cout << "Levenshteins Distance: " << maxDist << endl;

	// search
	file.open("E:\\LevenshteinsAutomata-master\\wordsToTest.txt");
	while (file.good())
	{
		file.getline(line, 256);
		// cout << "===================================================================" << endl;
		// cout << "Constructing Levenshteins Automata for word: " << line << endl;
		nfa = LevenshteinAutomata::NFA::getLevenshteinAutomata(line, maxDist);
		// cout << "Get LevenshteinAutomata NFA" << endl;
		// nfa->show();
		dfa = LevenshteinAutomata::DFA::getNFA2DFA(nfa);
		//cout << "Get LevenshteinAutomata DFA" << endl;
		// dfa->show();
		list<string> output;
		// cout << "Searching..." << endl;
		// cout << "DFA Start " << dfa->start << endl;
		timer = clock();
		dfa->search(&trie, dfa->start, trie.rootNode, output);
		duration = (clock() - timer) / (double)CLOCKS_PER_SEC;
		totalTime += duration;

		cout << "===================================================================" << endl;
		cout << "Constructing Levenshteins Automata for word: " << line << endl;
		cout << "Searching..." << endl;
		cout << "Construction and Search complete! Time: " << duration << " Seconds" << endl;
		cout << "Number of hits found: " << output.size() << endl;
		for (string s : output)
			cout << s << " ";
		cout << endl;
	}
	
	cout << "===================================================================" << endl;
	cout << "Total time: " << totalTime << " Seconds.." << endl;

	//clean Up
	delete nfa;
	delete dfa;
	// char c = getchar();
	return 0;
}*/

int main(int argc, char **argv)
{
	ifstream file;
	LevenshteinAutomata::Trie trie;
	clock_t timer;
	double duration = 0;
	double totalTime = 0;
	int maxDist = 2;
	LevenshteinAutomata::NFA* nfa;
	LevenshteinAutomata::DFA* dfa;
	char line[256];

	cout << "Constructing Candidate Keys Trie..." << endl;
	timer = clock();
	trie.rootNode = LevenshteinAutomata::getNode(' ', L"");
	// load in all resources
	vector<wstring> testList = { L"fxod", L"fd", L"sxood", L"f", L"fxd", L"sod", L"sood" };
	/*vector<string> testList = { "习j平", "习大大", "习", "李近平", "李大大", "近平", "xjp" };*/
	for (wstring line : testList)
		trie.insert(line);

	duration = (clock() - timer) / (double)CLOCKS_PER_SEC;
	cout << "Constructing Trie Complete! Time: " << duration << " Seconds" << endl;
	cout << "Levenshteins Distance: " << maxDist << endl;

	// search
	// cout << "===================================================================" << endl;
	// cout << "Constructing Levenshteins Automata for word: " << line << endl;
	nfa = LevenshteinAutomata::NFA::getLevenshteinAutomata(L"food", maxDist);
	// cout << "Get LevenshteinAutomata NFA" << endl;
	// nfa->show();
	dfa = LevenshteinAutomata::DFA::getNFA2DFA(nfa);
	//cout << "Get LevenshteinAutomata DFA" << endl;
	// dfa->show();
	list<wstring> output;
	// cout << "Searching..." << endl;
	// cout << "DFA Start " << dfa->start << endl;
	timer = clock();
	dfa->search(&trie, dfa->start, trie.rootNode, output);
	duration = (clock() - timer) / (double)CLOCKS_PER_SEC;
	totalTime += duration;

	cout << "===================================================================" << endl;
	cout << "Constructing Levenshteins Automata for word: " << line << endl;
	cout << "Searching..." << endl;
	cout << "Construction and Search complete! Time: " << duration << " Seconds" << endl;
	cout << "Number of hits found: " << output.size() << endl;
	for (wstring s : output)
		wcout << s << " ";
	cout << endl;

	cout << "===================================================================" << endl;
	cout << "Total time: " << totalTime << " Seconds.." << endl;

	//clean Up
	delete nfa;
	delete dfa;
	// char c = getchar();
	return 0;
}
