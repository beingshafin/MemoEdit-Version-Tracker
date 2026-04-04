#include <bits/stdc++.h>
#include <conio.h>
#include <windows.h>

using namespace std;

// colors
#define TITLE   "\033[1m\033[38;2;130;170;255m" //CYAN
#define BOLD    "\033[1m"
#define YELLOW "\033[33m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"
#define RED "\033[31m"
#define RESET  "\033[0m"

// macro functions
#define CLEAR_TO_END "\x1b[J" // clears everything from cursor to bottom
#define MOVE_CURSOR(y,x) cout << "\x1b[" << (y)+1 << ";" << (x)+1 << "H" // moves cursor to specific coord

// utilty functions (windows specific) --->
void clearScreen() {
    printf("\033[2J\033[H");
    fflush(stdout);
}

void enableAnsi() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= 0x0004;
    SetConsoleMode(hOut, dwMode);
}


//----------------------------------------------
// version node strcuture --->
struct VersionNode {
    int versionId;  // index of current version
    string message; // commit message
    string content; // actual document (text)
    VersionNode* prev;
    VersionNode* next;
};

// initiliazing variables globally --->
VersionNode* head = NULL;
VersionNode* tail = NULL;
VersionNode* current = NULL; // current version pointer

int versionCounter = 0; // version counter
string workingContent = ""; // current default buffer text 


//------------------------------------------------
// Linked List Functions ---> (NO STL)

// delete future versions when reverted to a past commit and done a new commit
void deleteFutureVersions(VersionNode* node) { // input current
    VersionNode* ptr= node->next;

    while (ptr != NULL) {
        VersionNode* nextNode= ptr->next;
        delete ptr;
        ptr= nextNode;
    }

    node->next = NULL;
    tail = node;
}


// passing the commit message with buffer content (global)
void commitWorkingContent(string msg) {
    
    // case 0 -> no changes
    // If buffer is empty or there's no change
    if (workingContent.empty()) {
        cout << "Nothing to commit (content is empty).\n";
        return;
    }
    if (current != NULL && workingContent == current->content) {
        cout << "Nothing to commit.\n";
        return;
    }
    
    // ~~node creation
    VersionNode* newNode = new VersionNode; // new is similar to malloc but better for c++ as we are using strings
    newNode->message = msg;
    newNode->content = workingContent;
    newNode->prev = NULL;
    newNode->next = NULL;
    
    // case 1 -> first commit
    if(head == NULL){ // empty history so this commit is everything
        newNode->versionId = ++versionCounter;
        head = tail = current = newNode;
        return;
    }
    
    
    // case 2 -> normal commit
    if(current != tail){ // if on reverted node delete future versions
        deleteFutureVersions(current);
    }
    
    versionCounter = current->versionId; // required after deletion of future ver
    newNode->versionId = ++versionCounter; 
    tail->next = newNode; // current->next also works
    newNode->prev = tail; // = current also works
    tail = newNode;
    current = newNode;
    
    
}



// moving the current pointer to a prev version
void revertToVersion(int id) {
    
    for(auto ptr=head; ptr!=NULL; ptr =ptr->next) {
        if (ptr->versionId == id) {
            current = ptr;
            workingContent = ptr->content;
            return;
        }
    }

    cout << "Version not found.\n";
}

// printing history with arrow on current version
void showHistory(){
    if(!head){
        cout << "\nNo versions available.\n";
        return ;
    }

    for(auto ptr=head; ptr; ptr=ptr->next){
        ptr == current ? cout << "-> " : cout << "   ";
        cout << "[Version " << ptr->versionId << "] " << ptr->message << endl;
    }
}



void runEditor() {
    clearScreen();

    // console variables (ALL HAIL GPT)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE); // handle to console output
    CONSOLE_SCREEN_BUFFER_INFO csbi; // console screen buffer info
    GetConsoleScreenBufferInfo(hOut, &csbi); // get console screen buffer info

    SHORT screenWidth = 0, startX = 0, startY = 0; 

    auto drawUI = [&]() {
        GetConsoleScreenBufferInfo(hOut, &csbi); 
        screenWidth = csbi.dwSize.X;

        // print text centered
        auto printCenteredLine = [&](int row, string text) {
            int padding = screenWidth - 2 - text.size();
            int leftPadding = max(0, padding / 2);
            int rightPadding = max(0, padding - leftPadding);
            MOVE_CURSOR(row, 0);
            cout << "|" << string(leftPadding, ' ') << text << string(rightPadding, ' ') << "|";
        };
        
        MOVE_CURSOR(0, 0); cout << BLUE << "+" << RESET << string(screenWidth - 2, '-') << BLUE << "+" << RESET;


        // printing title with colors
        int padding = screenWidth - 2 - string("INLINE EDITOR").size();
        int leftPadding = max(0, padding / 2);
        int rightPadding = max(0, padding - leftPadding);
        MOVE_CURSOR(1, 0);
        cout << "|" << string(leftPadding, ' ') << BLUE << "INLINE EDITOR" << RESET << string(rightPadding, ' ') << "|";
        
        // printing shortcuts
        printCenteredLine(2, "[Ctrl+Y] Save Memory        [Ctrl+D] Commit");
        printCenteredLine(3, "[Ctrl+Z] Undo               [Ctrl+R] Redo  ");
        printCenteredLine(4, "[Ctrl+E] Exit");
        MOVE_CURSOR(5, 0); cout << BLUE << "+" << RESET << string(screenWidth - 2, '-') << BLUE << "+" << RESET;
        MOVE_CURSOR(6, 0); cout << GREEN << "'''" << RESET << endl;
        //MOVE_CURSOR(7, 0); cout << GREEN << "'''" << RESET << endl;
        
        startX = 0;
        startY = 7; // starting at 8th line
    };

    string buffer = workingContent; // buffer
    int index = buffer.size(); // cursor position index

    vector<string> history = {buffer}; // history of buffer
    int historyPtr = 0; // current history position pointer

    
    auto moveCursor = [&](int idx){ // convert linear index to screen coordinates and moves to it
        int row = startY;
        int col = startX;
        
        for(int i=0; i<idx; i++){
            if(buffer[i] == '\n' || col >= screenWidth-1){ // if newline or end of screen
                row++;
                col = 0;
            }
            else col++;
        }
        
        MOVE_CURSOR(row, col);
    };
    
    auto redraw = [&](int fromIndex){
        moveCursor(fromIndex);
        cout << CLEAR_TO_END;
        cout << buffer.substr(fromIndex);
        moveCursor(index); // restoring cursor to buffers end
    };
    
    auto pushHistory = [&](){
        history.erase(history.begin() + historyPtr+1, history.end()); // if on prev history erase forward history
        history.push_back(buffer);
        historyPtr++;
    };

    auto showStatus = [&](string msg){ // centered padding
        MOVE_CURSOR(6, 0); // Inside box, top line (row 1, col 2 -> 2nd row, 3rd char? box starts at row 0?)
        cout << YELLOW << msg << RESET;
        moveCursor(index); // restore cursor to buffer size
    };
    
    // initial ui & buffer display
    drawUI();
    cout << buffer;
    moveCursor(index);

    while(true){
        // dynamic window resize while waiting for key press
        while (!_kbhit()) {
            GetConsoleScreenBufferInfo(hOut, &csbi);
            if (csbi.dwSize.X != screenWidth) {
                clearScreen();
                drawUI();
                redraw(0);
            }
            Sleep(10);
        }

        char ch = _getch(); // getch() returns int
        if (ch == 5) break; // ctrl+e exits
        else if (ch >= 32 && ch <= 126) { // printable characters (visible ascii)
            buffer.insert(index, 1, ch);
            index++;
            pushHistory();
            redraw(index - 1); // print the current buffer substr from prev index and moves cursor to current index
            showStatus("EDITING...       ");
        }
        else if (ch == 8 && index > 0) { // Backspace
            buffer.erase(index - 1, 1);
            index--;
            pushHistory();
            redraw(index);
            showStatus("EDITING...       ");
        }
        else if (ch == 13) { // Enter key
            buffer.insert(index, 1, '\n');
            index++;
            pushHistory();
            redraw(index - 1);
            showStatus("EDITING...       ");
        }
        // navigation keys (special keys) -> no redraw, only move <-->
        else if (ch == -32 || ch == 0) { // first ch indicates special key
            char special = _getch(); // 2nd ch gives the actual code

            if (special == 75 && index > 0) index--; // left
            else if (special == 77 && index < buffer.size()) index++; // right
            else if (special == 71) index = 0; // home
            else if (special == 79) index = buffer.size(); // end

            // redraw xd
            else if (special == 83 && index < buffer.size()) { // Delete
                buffer.erase(index, 1);
                pushHistory();
                redraw(index);
                showStatus("EDITING...       ");
            }

            moveCursor(index); // reset cursor pos to buffer size
        }

        // no push history, just moving through it <-->
        else if( ch == 26 && historyPtr>0) { // ctrl+z (undo)
            historyPtr--; // now points to prev history idx
            buffer = history[historyPtr]; // updating buffer
            index = buffer.size();
            redraw(0); // entire redraw (cus we dont know how big the changes are)
            showStatus("UNDO             ");
        }
        else if (ch == 18 && historyPtr + 1 < history.size()) { // Ctrl+R (Redo)
            buffer = history[++historyPtr];
            index = buffer.size();
            redraw(0);
            showStatus("REDO             ");
        }

        if (ch == 25) { // Ctrl+Y (Save to Memory)
            string lastCommitContent = (current == NULL) ? "" : current->content;
            if(buffer == lastCommitContent){
                showStatus("NOTHING TO SAVE ");
            }
            else{
                workingContent = buffer;
                showStatus("SAVED TO MEMORY! ");
            }
        }

        // commit 
        else if (ch == 4) { // Ctrl+D (Commit / Finish and Name this version)
            if (buffer.empty() || (current != NULL && buffer == current->content)) {
                showStatus("NOTHING TO COMMIT ");
                continue;
            }

            workingContent = buffer;

            //clearScreen();
            
            MOVE_CURSOR(6, 0);
            cout << GREEN << "COMMIT MESSAGE: " << RESET;
            string msg;
            getline(cin, msg);
            if (msg.empty()) msg = "No message";
            msg = msg.substr(0, 25);
            
            commitWorkingContent(msg);

            // Return to editor instead of exiting to menu
            clearScreen();
            drawUI();
            redraw(0);
            showStatus("COMMITTED!        ");
        }
    }
}

void freeAll() {
    VersionNode* temp = head;
    while (temp) {
        VersionNode* nextNode = temp->next;
        delete temp;
        temp = nextNode;
    }
}
    
int main(){
    enableAnsi();
    int choice;

    while(1){
        clearScreen();

        cout << "\n";
        cout << GREEN << "< MEMOxEDIT V.Tracker >\n\n" << RESET;
        cout << CYAN << " [1] " << RESET << "Edit\n";
        cout << CYAN << " [2] " << RESET << "Commit\n";
        cout << CYAN << " [3] " << RESET << "Revert\n";
        cout << CYAN << " [4] " << RESET << "History\n";
        cout << CYAN << " [0] " << RESET << "Exit\n\n"; 
        
        if ((current != NULL && workingContent != current->content) || (current == NULL && workingContent != "")) cout << RED << "Uncommited changes detected\n" << RESET;
        
        cout << BLUE << "Enter choice: " << RESET;
        cin >> choice;
        cin.ignore(); // prevent newline from breaking getline later;
        if(choice==1){
            runEditor();
        }
        else if(choice==2){
            cout << GREEN << "Commit message: " << RESET;
            string msg;
            getline(cin, msg);
            if(msg.empty()) msg = "No Message";
            commitWorkingContent(msg);
            system("pause");
        }
        else if(choice==3){
            cout << "Enter version Number: ";
            int id;
            cin >> id;
            cin.ignore();
            revertToVersion(id);
            system("pause");
        }
        else if(choice==4){
            showHistory();
            system("pause");
        }
        else if(choice==0){
            freeAll();
            cout << "Exiting..\n";
            cout << "Daripallay Vote din\n";
            return 0;
        }
        else {
            cout << "Invalid choice.\n";
            system("pause");
        }
    }

    return 0;
}
