#include <bits/stdc++.h>

using namespace std;
const int guest_amount = 500000;
const int threadN = 4;
const int MIN = numeric_limits<int>::min();
const int MAX = numeric_limits<int>::max();



vector<thread> threads; //will be four as per pdf
vector<int> bag(guest_amount);//this will make a bag of the size of the amount of guests
atomic<int> c = 0; //counter will be used later for threads which is why
atomic<int> ic, rc;// will have a counter of insert counter and remove so when we make bag we can swap the order of the elements

// linked list node with value, next, and a mutex
struct node {
  int v; //v will represent value that will be currently accessed
  node *next; //next pointer
  mutex m; //normal mutex

  node(int v): v(v) {}
};


class LockOrderedLinkedList {
private:
  node *head; 

public:
  LockOrderedLinkedList() {
    head = new node(MIN); //initializing  to value that can be replaced at start
    head->next = new node(MAX); //initializing
  }

  void insert(int v) {
    node *cur = new node(v); // create the new node which will be our current 

    node *prev = head; //set previous to head
    unique_lock<mutex> prevLock(prev->m); //locks it so no other threads can access it
    node *next = prev->next;//same thing as above but for next
    unique_lock<mutex> nextLock(next->m);

    //this while loop is meant to run while the value that is next is smaller than current in herer we will sort if needed
    while(v > next->v) {
      prev = next;
      prevLock.swap(nextLock);
      next = next->next;
      nextLock = unique_lock<mutex>(next->m);
    }

    //this is where we just insert
    prev->next = cur;
    cur->next = next;
  }

  void remove(int value) {
    //same concept as the insert just following textbook concept for removing
    node *prev = head;
    unique_lock<mutex> prevLock(prev->m);
    node *curr = prev->next;
    unique_lock<mutex> nodeLock(curr->m);
    node *next = curr->next;
    unique_lock<mutex> nextLock(next->m);

    //this while loop will also sort for us until we get to value that we need to actually remove
    while(value > curr->v) {
      prev = curr;
      prevLock.swap(nodeLock);
      curr = next;
      nodeLock.swap(nextLock);
      // this line will only apply if nothing has yet been added meaning no need to remove
      if(curr->v == MAX) {
        return;
      }
      next = next->next;
      nextLock = unique_lock<mutex>(next->m);
    }

    // this will remove if we find our value
    if(curr->v == value) {
      prev->next = next;
    }
  }

  bool search(int value) {
    node *node = head;
    unique_lock<mutex> nodeLock(node->m);

    // this while loop will traverse the list until we find the correct element we are looking for
    while(value < node->v) {
      node = node->next;
      auto nextLock = unique_lock<mutex>(node->m);
      nodeLock.swap(nextLock);
    }

    return node->v == value;
  }
};

LockOrderedLinkedList linkedList;
//servant is our threads
void servant(int id) {

  int iORd = 0;//this variable will let us know wether we need to insert or delete
  //while loop below will run until the whole bag is empty based on our counter and amount of guests
  while(ic < guest_amount) {
    

    // insert
    if(iORd == 0) {
      if(rc == guest_amount) continue;//if this line is true that means we removed all presents so no need to insert
      int element = rc++;
      int v = bag[element];
      cout << "Servant " << id <<" added present "<< v << " in its rightful ordered place" <<endl;
      linkedList.insert(v);
      
      c++;
      
      iORd ^= 1;//once we insert switch the number to delete operation this is to simulate that servants can insert and delete 
                //without waiting for all to be inserted
      
      continue;
    }

    // remove
    if(iORd == 1) {
      if(ic == rc) continue;//if both counters are the same this means we are done with inserting and removing/making thank you cards.
      int element = ic++;
      int v = bag[element];
      linkedList.remove(v);
      cout << "Servant " << id <<" made a thank you card for guest "<< v << " removed the present " << "and made sure the chain is still ordered" <<endl;

      //same thing as with insert for the insert or delete variable
      iORd ^= 1;
      continue;
    }
  }
}


int main() {

  // rng generator
  srand(time(0));
  //makes the bag list as big as the amount of guests
  for(int i = 0; i < guest_amount; i++) {
    bag[i] = i + 1;
  }
  //the for loop below will unordered our bag 
  for(int i = 0; i < 500000; i++) {
    swap(bag[rand() % guest_amount], bag[rand() % guest_amount]);
  }
  

  linkedList = LockOrderedLinkedList();

  for(int i = 0; i < threadN; i++) {
    threads.push_back(thread(servant, i));
  }
  for(int i = 0; i < threadN; i++) {
    threads[i].join();
  }
  
  cout << "task done" << endl;
  

  return 0;
}

