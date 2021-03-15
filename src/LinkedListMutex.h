#include <iostream>

using namespace std;

struct node
{
    int data;
    node *next;
};

class LinkedListMutex
{
private:
    node *head, *tail;

public:
    LinkedListMutex()
    {
        head = NULL;
        tail = NULL;
    }

    bool search(int n)
    {
        node *current = head;

        while (current != NULL && current->data != n)
        {
            current = current->next;
        }
        if (current != NULL)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool add(int n)
    {
        //create new node
        node *newNode = new node;
        newNode->data = n;
        newNode->next = NULL;

        //List is empty
        if (head == NULL)
        {
            head = newNode;
            tail = newNode;
            return true;
        }

        //Value is less than head
        if (head->data >= n)
        {
            newNode->next = head;
            head = newNode;
            return true;
        }

        //traverse the list to find the where the new node goes
        node *current = head;
        node *previous = current;
        while (current != NULL && current->data <= n)
        {
            previous = current;
            current = current->next;
        }

        //insert the new node into the list between the previous and current
        previous->next = newNode;
        newNode->next = current;
        if (newNode->next == NULL)
        {
            tail = newNode;
        }
        return true;
    }

    void display()
    {
        node *current = head;
        while (current != NULL)
        {
            cout << current->data << " " << current->next << endl;
            current = current->next;
        }
    }

    bool remove(int n)
    {
        // If the head is to be deleted
        if (head->data == n)
        {
            node *previousHead = head;
            head = head->next;
            delete previousHead;
            return true;
        }

        // If there is only one element in the list
        if (head->next == NULL)
        {
            // If the head is to be deleted. Assign null to the head
            if (head->data == n)
            {
                node *previousHead = head;
                head = NULL;
                delete previousHead;
                return true;
            }
            // else value not found
            return false;
        }

        // Else loop over the list and search for the node to delete
        node *temp = head;
        while (temp->next != NULL)
        {
            // When node is found, delete the node and modify the pointers
            if (temp->next->data == n)
            {
                node *temp_ptr = temp->next->next;
                delete temp->next;
                temp->next = temp_ptr;
                return true;
            }
            temp = temp->next;
        }

        // else value not found
        return false;
    }
};