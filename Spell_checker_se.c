#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define LENGTH 45 // maximum length for word
#define N 26

// Define a trie node structure
typedef struct node {
    bool isWord; // Flag to indicate if the node represents a complete word
    struct node* children[N]; // Array of pointers to child nodes
} node;

node* root; // Global variable to store the root of the trie
int _size = 0; // Global variable to store the size of the trie (number of words)

// Function to load words from a file into the trie
bool load() {
    // Allocate memory for the root node
    root = malloc(sizeof(node));
    if (root == NULL) return false;

    root->isWord = false; // Initialize root node
    for (int i = 0; i < N; i++) {
        root->children[i] = NULL;
    }

    // Open the file containing the dictionary words
    FILE* file = fopen("words.txt", "r");
    if (file == NULL) return false;

    char word[LENGTH + 1];
    int idx;

    // Read words from the file and insert them into the trie
    while (fscanf(file, "%s", word) != EOF) {
        node* child = root;
        for (int i = 0, len = strlen(word); i < len; i++) {
            idx = (int)word[i] - (int)'a';

            if (child->children[idx] == NULL) {
                // Allocate memory for a new node if it doesn't exist
                child->children[idx] = malloc(sizeof(node));
                if (child->children[idx] == NULL) return false;

                // Initialize the new node
                child->children[idx]->isWord = false;
                for (int j = 0; j < N; j++) {
                    child->children[idx]->children[j] = NULL;
                }
            }
            child = child->children[idx];
        }
        child->isWord = true; // Mark the last node as representing a complete word
        _size++; // Increment the word count
    }

    fclose(file); // Close the file
    return true;
}

// Function to return the size of the trie (number of words)
int size(void) {
    return _size;
}

// Function to check if a word is in the trie
bool check(const char* word) {
    int idx;
    node* child = root;
    for (int i = 0, len = strlen(word); i < len; i++) {
        idx = (int)tolower(word[i]) - (int)'a';

        child = child->children[idx];
        if (child == NULL) return false;
    }
    return child->isWord;
}

// Function to recursively unload nodes from memory
void unloadNode(node* top) {
    if (top == NULL) return;

    for (int i = 0; i < N; i++) {
        if (top->children[i] != NULL) {
            unloadNode(top->children[i]);
        }
    }
    free(top);
}

// Function to unload the trie from memory
void unload() {
    unloadNode(root);
}

// Function to calculate the minimum of three integers
int min(int a, int b, int c) {
    return a < b ? (a < c ? a : c) : (b < c ? b : c);
}

// Function to calculate the edit distance between two words
int editDistance(const char* word1, const char* word2) {
    int len1 = strlen(word1);
    int len2 = strlen(word2);

    int dp[len1 + 1][len2 + 1];

    for (int i = 0; i <= len1; i++) {
        for (int j = 0; j <= len2; j++) {
            if (i == 0) dp[i][j] = j;
            else if (j == 0) dp[i][j] = i;
            else if (word1[i - 1] == word2[j - 1]) dp[i][j] = dp[i - 1][j - 1];
            else dp[i][j] = 1 + min(dp[i - 1][j - 1], dp[i - 1][j], dp[i][j - 1]);
        }
    }

    return dp[len1][len2];
}

// Function to suggest corrections for a misspelled word
void suggest(const char* word) {
    // Check if the original word is a valid word
    if (check(word)) {
        printf("- %s\n", word);
        return;
    }

    // Traverse the trie to find similar words with edit distance <= 2
    printf("\033[1;33mSuggestions for\033[0m \"\033[1;31m%s\033[0m\":\n", word);
    char suggestion[LENGTH + 1];

    // Try deleting one character from the original word
    for (int i = 0; i <= strlen(word); i++) {
        strcpy(suggestion, word);
        memmove(&suggestion[i], &suggestion[i + 1], strlen(suggestion) - i);
        if (check(suggestion)) {
            printf("- \033[1;32m %s\033[0m\n", suggestion);
        }
    }

    // Try inserting one character into the original word
    for (int i = 0; i <= strlen(word); i++) {
        for (char c = 'a'; c <= 'z'; c++) {
            strcpy(suggestion, word);
            memmove(&suggestion[i + 1], &suggestion[i], strlen(suggestion) - i + 1);
            suggestion[i] = c;
            if (check(suggestion)) {
                printf("-\033[1;32m %s\033[0m\n", suggestion);
            }
        }
    }

    // Try replacing one character in the original word
    for (int i = 0; i < strlen(word); i++) {
        for (char c = 'a'; c <= 'z'; c++) {
            strcpy(suggestion, word);
            suggestion[i] = c;
            if (check(suggestion)) {
                printf("- \033[1;32m %s\033[0m\n", suggestion);
            }
        }
    }
}

// Main function
int main() {
    // Load the dictionary into the trie
    if (!load()) {
        printf("Could not load dictionary\n");
        return 1;
    }

    // Input sentence from user
    printf("===================================================================================================\n");
    printf("\t\t\t\t\tSpell Checker\n");
    printf("===================================================================================================\n");
    printf("Enter the sentence to be checked (Only Characters):");

    char sentence[100];
    fgets(sentence, sizeof(sentence), stdin);

    printf("\n==================\n");
    printf("\033[1;31mMISSPELLED WORDS:\033[0m\n");
    printf("==================\n");
    
    int index = 0, misspellings = 0, words = 0;
    char word[LENGTH + 1];

    // Tokenize the sentence into words and check for misspellings
    for (int i = 0, len = strlen(sentence); i < len; i++) {
        char c = sentence[i];
        if (isalpha(c)) {
            word[index] = c;
            index++;

            if (index > LENGTH) {
                while (i < len && isalpha(sentence[i])) {
                    i++;
                }
                index = 0;
            }
        }
        else if (isdigit(c)) {
            while (i < len && isalnum(sentence[i])) {
                i++;
            }
            index = 0;
        }
        else if (index > 0) {
            word[index] = '\0';
            index++;
            words++;

            // Check if the word is misspelled and suggest corrections
            if (!check(word)) {
                printf("\033[1;31m%s\033[0m\n", word);
                
                suggest(word);
                misspellings++;
            }
            index = 0; // Reset index for the next word
        }
    }

    unload(); // Unload the trie from memory

    // Print statistics
    printf("\nDictionary word count: %d", size());
    printf("\nSentence word count:       %d", words);
    printf("\nMisspelled words:      %d\n", misspellings);
    printf("===================================================================================================\n");
    return 0;
}
