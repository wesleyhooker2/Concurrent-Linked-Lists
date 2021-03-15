#include <iostream>
#include <cstdio>
#include <atomic>

using namespace std;
using std::atomic;
using std::atomic_compare_exchange_strong;

class Node
{
public:
    int data;
    atomic<Node *> next{nullptr};

public:
    Node(int n)
    {
        data = n;
    }
};

Node *get_unmarked_reference(Node *ptr)
{
    uint64_t val = reinterpret_cast<std::uintptr_t>(ptr);
    if (val % 2 == 1)
    {
        val -= 1;
    }
    return reinterpret_cast<Node *>(val);
}

Node *get_marked_reference(Node *ptr)
{
    uint64_t val = reinterpret_cast<std::uintptr_t>(ptr);
    if (val % 2 == 0)
    {
        val += 1;
    }
    return reinterpret_cast<Node *>(val);
}

bool is_marked_reference(Node *ptr)
{
    uint64_t val = reinterpret_cast<std::uintptr_t>(ptr);
    return (val % 2 == 1);
}

class LinkedListAtomic
{
private:
    Node *head, *tail;

    Node *search(int n, Node **left_node)
    {
        Node *left_node_next, *right_node;

        do
        {
            Node *t = head;
            Node *t_next = head->next;

            //1 Find left and right node
            do
            {
                if (!is_marked_reference(t_next))
                {
                    (*left_node) = t;
                    left_node_next = t_next;
                }
                t = get_unmarked_reference(t_next);
                if (t == tail)
                {
                    break;
                }
                t_next = t->next;
            } while (is_marked_reference(t_next) || (t->data < n));
            right_node = t;

            //2 Check nodes are adjacent
            if (left_node_next == right_node)
            {
                if ((right_node != tail) && is_marked_reference(right_node->next))
                {
                    continue;
                }
                else
                {
                    return right_node;
                }
            }

            //3 Remove one or more marked nodes
            if (atomic_compare_exchange_strong(&(*left_node)->next, &left_node_next, right_node))
            {
                if ((right_node != tail) && is_marked_reference(right_node->next))
                {
                    continue;
                }
                else
                {
                    return right_node;
                }
            }
        } while (true);
    }

public:
    LinkedListAtomic()
    {
        head = new Node(0);
        tail = new Node(100);
        head->next = tail;
    }

    bool insert(int n)
    {
        Node *new_node = new Node(n);
        Node *right_node, *left_node;

        do
        {
            right_node = search(n, &left_node);
            if ((right_node != tail) && (right_node->data == n))
            {
                return false;
            }
            new_node->next = right_node;
            if (atomic_compare_exchange_strong(&(left_node->next), &right_node, new_node))
            {
                return true;
            }
        } while (true);
    }

    bool find(int n)
    {
        Node *right_node, *left_node;
        right_node = search(n, &left_node);
        if ((right_node == tail) ||
            (right_node->data != n))
            return false;
        else
            return true;
    }

    bool remove(int n)
    {
        Node *right_node, *right_node_next, *left_node;
        do
        {
            right_node = search(n, &left_node);
            if ((right_node == tail) || (right_node->data != n))
            {
                return false;
            }
            right_node_next = right_node->next;

            if (!is_marked_reference(right_node_next))
            {
                if (atomic_compare_exchange_strong(&(right_node->next),
                                                   &right_node_next, get_marked_reference(right_node_next)))
                {
                    break;
                }
            }
        } while (true);

        if (!atomic_compare_exchange_strong(&(left_node->next), &right_node, right_node_next))
        {
            right_node = search(right_node->data, &left_node);
        }
        return true;
    }

    void display()
    {
        Node *current = head;
        while (current != NULL)
        {
            cout << current->data << " " << current->next << endl;
            current = current->next;
        }
    }
};