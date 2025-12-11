#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <stack>
#include <unordered_map>
#include <algorithm>
#include <cctype>
using namespace std;

// Flashcard object
struct Flashcard {
    string term;
    string definition;
    string category;
};

// Category node for tree
struct CategoryNode {
    string name;
    vector<CategoryNode*> children;
    vector<int> cardIndices;
};

vector<Flashcard> allCards;
queue<int> quizQueue;
stack<int> reviewStack;
unordered_map<string, int> indexByTerm;
unordered_map<int, vector<int>> relatedGraph;
CategoryNode* rootCategory = nullptr;

// Lowercase so it's easier to check
string ToLower(const string& s) {
    string r = s;
    transform(r.begin(), r.end(), r.begin(),
             [](unsigned char c){ return (char)tolower(c); });
    return r;
}

// Build category tree
CategoryNode* BuildCategoryTree() {
    CategoryNode* root = new CategoryNode{"All Topics"};
    CategoryNode* cs = new CategoryNode{"CS Concepts"};
    CategoryNode* misc = new CategoryNode{"Misc"};

    root->children.push_back(cs);
    root->children.push_back(misc);

    for (int i = 0; i < (int)allCards.size(); ++i) {
        if (allCards[i].category == "CS Concepts") {
            cs->cardIndices.push_back(i);
        }
        else {
            misc->cardIndices.push_back(i);
        }
    }
    return root;
}

// Print category tree
void PrintCategoryTree(CategoryNode* node, int depth = 0) {
    if (!node) return;

    for (int i = 0; i < depth; ++i) cout << "  ";
    cout << "- " << node->name << "\n";

    for (int idx : node->cardIndices) {
        for (int i = 0; i < depth + 1; ++i) cout << "  ";
        cout << "* " << allCards[idx].term << "\n";
    }
    for (CategoryNode* child : node->children) {
        PrintCategoryTree(child, depth + 1);
    }
}

// Build related graph (same category = related)
void BuildRelatedGraph() {
    int n = allCards.size();
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (allCards[i].category == allCards[j].category) {
                relatedGraph[i].push_back(j);
                relatedGraph[j].push_back(i);
            }
        }
    }
}

// Show related cards
void ShowRelatedCards(int index) {
    auto it = relatedGraph.find(index);
    if (it == relatedGraph.end() || it->second.empty()) {
        cout << "  No related cards.\n";
        return;
    }
    cout << "  Related cards:\n";
    for (int n : it->second) {
        cout << "    - " << allCards[n].term << " (" << allCards[n].category << ")\n";
    }
}

// Start program with empty deck
void InitializeDeck() {
    allCards.clear();
    indexByTerm.clear();
}

// Fill quiz queue
void PrepareQuiz() {
    while (!quizQueue.empty()) quizQueue.pop();
    for (int i = 0; i < (int)allCards.size(); i++) {
        quizQueue.push(i);
    }
}

// Ask a question
bool Ask(int idx) {
    const Flashcard& c = allCards[idx];

    cout << "\nTerm: " << c.term << "\n";
    cout << "Enter definition (or 'show' if you don't know):\n> ";

    string ans;
    getline(cin, ans);

    if (ToLower(ans) == "show") {
        cout << "Correct: " << c.definition << "\n";
        return false;
    }
    if (ToLower(ans) == ToLower(c.definition)) {
        cout << "Correct!\n";
        return true;
    }
    cout << "Incorrect.\nCorrect: " << c.definition << "\n";
    return false;
}

// Run quiz
void StartQuiz() {
    if (quizQueue.empty()) {
        cout << "No cards in quiz. Prepare quiz first.\n";
        return;
    }

    while (!reviewStack.empty()) reviewStack.pop();

    cout << "\n--- Quiz Started ---\n";

    while (!quizQueue.empty()) {
        int idx = quizQueue.front();
        quizQueue.pop();

        bool correct = Ask(idx);
        if (!correct) reviewStack.push(idx);

        ShowRelatedCards(idx);
    }

    cout << "\nQuiz complete.\n";
    if (reviewStack.empty()) {
        cout << "All correct.\n";
    }
    else {
        cout << reviewStack.size() << " incorrect. Use review option.\n";
    }
}

// Review stack items
void ReviewMissed() {
    if (reviewStack.empty()) {
        cout << "No missed cards.\n";
        return;
    }

    cout << "\nReview Missed\n";

    while (!reviewStack.empty()) {
        int idx = reviewStack.top();
        reviewStack.pop();

        cout << "\nReview: " << allCards[idx].term << "\n";
        bool ok = Ask(idx);
        if (!ok) cout << "Try to remember this.\n";
    }
    cout << "Review finished.\n";
}

// Look up term
void Lookup() {
    cout << "Enter term: ";
    string t;
    getline(cin, t);

    auto it = indexByTerm.find(t);
    if (it == indexByTerm.end()) {
        cout << "Not found.\n";
        return;
    }

    int idx = it->second;
    cout << "Definition: " << allCards[idx].definition << "\n";
    cout << "Category: " << allCards[idx].category << "\n";

    ShowRelatedCards(idx);
}

// Create flashcard
void Create() {
    string term, def, cat;

    cout << "\nEnter term:\n> ";
    getline(cin, term);

    cout << "Enter definition:\n> ";
    getline(cin, def);

    cout << "Enter category:\n> ";
    getline(cin, cat);

    Flashcard c{term, def, cat};
    allCards.push_back(c);

    int index = allCards.size() - 1;
    indexByTerm[term] = index;

    bool placed = false;
    if (rootCategory != nullptr) {
        for (CategoryNode* child : rootCategory->children) {
            if (child->name == cat) {
                child->cardIndices.push_back(index);
                placed = true;
                break;
            }
        }
        if (!placed) {
            for (CategoryNode* child : rootCategory->children) {
                if (child->name == "Misc") {
                    child->cardIndices.push_back(index);
                    break;
                }
            }
        }
    }

    relatedGraph.clear();
    BuildRelatedGraph();

    cout << "Flashcard added.\n";
}

// Print menu
void Menu() {
    cout << "\n=== Flashcard Final Project ===\n";
    cout << "1. Show categories\n";
    cout << "2. Prepare quiz\n";
    cout << "3. Start quiz\n";
    cout << "4. Review missed\n";
    cout << "5. Look up term\n";
    cout << "6. Add flashcard\n";
    cout << "0. Exit\n";
    cout << "Choice: ";
}

int main() {
    InitializeDeck();
    rootCategory = BuildCategoryTree();
    BuildRelatedGraph();

    bool run = true;
    while (run) {
        Menu();
        string c;
        getline(cin, c);
        if (c.empty()) continue;

        int choice = c[0] - '0';

        switch (choice) {
            case 1:
                PrintCategoryTree(rootCategory);
                break;
            case 2:
                PrepareQuiz();
                cout << "Quiz prepared.\n";
                break;
            case 3:
                StartQuiz();
                break;
            case 4:
                ReviewMissed();
                break;
            case 5:
                Lookup();
                break;
            case 6:
                Create();
                break;
            case 0:
                run = false;
                break;
            default:
                cout << "Invalid.\n";
                break;
        }
    }
    return 0;
}
