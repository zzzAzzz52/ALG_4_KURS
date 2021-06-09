#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <limits>
#include "List_of_lists.h"

using namespace std;

class OrGraph
{
    Adj_List* adj_list;
    int length;
    int* E;
    int* H;
    List* queue;

    void LIFT(Adj_List_Elem* Elem)
    {
        ElemL2* CurRib = Elem->inf->at(0);
        Adj_List_Elem* To;
        int min = INT_MAX;
        int temp;
        while (CurRib)
        {
            if (CurRib->key > 0)
            {
                To = adj_list->at(CurRib->inf);
                temp = H[adj_list->get_index_of(To->name)];
                if (min > temp)
                    min = temp;
            }
            CurRib = CurRib->next;
        }

        int index_Elem = adj_list->get_index_of(Elem->name);
        H[index_Elem] = (min + 1);
    }

    bool PUSH(Adj_List_Elem* From, Adj_List_Elem* To)
    {
        int index_from = adj_list->get_index_of(From->name);
        int index_to = adj_list->get_index_of(To->name);

        if (H[index_to] != (H[index_from] - 1))
            return false;

        ElemL2* Cur1 = From->inf->find_by_inf(index_to);
        int e = E[index_from];
        int rib = Cur1->key;
        int min = (rib > e) ? e : rib;

        ElemL2* Cur2 = To->inf->find_by_inf(index_from);
        E[index_from] -= min;
        E[index_to] += min;
        Cur1->key -= min;
        Cur2->key += min;

        return true;
    }

    bool DISCHARGE(Adj_List_Elem* Elem)
    {
        int index_elem = adj_list->get_index_of(Elem->name);
        ElemL2* Cur;
        bool lifted = false;

        while (E[index_elem] != 0)
        {
            Cur = Elem->inf->at(0);
            while (Cur)
            {
                if ((Cur->key > 0) && E[index_elem] != 0)
                    PUSH(Elem, adj_list->at(Cur->inf));
                Cur = Cur->next;
            }
            if (E[index_elem] != 0)
            {
                LIFT(Elem);
                lifted = true;
            }
        }
        return lifted;
    }

    void Init()
    {
        Adj_List_Elem* S = adj_list->find("S");
        int s_index = adj_list->get_index_of("S");
        H[s_index] = length;

        ElemL2* Cur = S->inf->at(0);
        ElemL2* OpCur;

        while (Cur)
        {
            OpCur = adj_list->at(Cur->inf)->inf->find_by_inf(s_index);
            OpCur->key = Cur->key;
            E[Cur->inf] = Cur->key;
            Cur->key = 0;
            Cur = Cur->next;
        }

        Adj_List_Elem* CurAdjElem = adj_list->at(0);
        for (int i = 0; i < length; i++)
        {
            if (CurAdjElem->name != "S" && CurAdjElem->name != "T")
                queue->push_back(i, 0);
            CurAdjElem = CurAdjElem->next;
        }
    }

    bool crt_rib(string name1, string name2, string rib)
    {
        if (rib.empty() || name2.empty())
            return 0;

        int weight;
        try { weight = stoi(rib); }
        catch (exception) { return 0; }
        if (weight < 0)
            return 0;

        bool ret = adj_list->create_rib(name1, name2, weight);
        if (ret)
            ret = adj_list->create_rib(name2, name1, 0);
        return ret;
    }

    bool string_working(ifstream* input)
    {
        string cur;
        getline(*input, cur);
        if (cur.empty())
            return 0;

        size_t p1 = 0, p2 = 0;
        string name1, name2, rib;

        for (int i = 0; i < cur.length(); i++)
        {
            if (cur[i] == ';')
            {
                if (p1 == 0)
                    p1 = i;
                else if (p2 == 0)
                    p2 = i;
                else
                    return 0;
            }
        }
        if (p1 == 0 || p2 == 0)
            return 0;

        rib = cur.substr(p2 + 1);
        cur.erase(p2);
        name2 = cur.substr(p1 + 1);
        cur.erase(p1);
        name1 = cur;

        if (!crt_rib(name1, name2, rib))
            return 0;
        return 1;
    }

    void input(string filename)
    {
        ifstream input;
        input.open(filename);
        if (!input.is_open())
            throw runtime_error("Could not open file " + filename);
        while (!input.eof())
        {
            if (!string_working(&input))
            {
                input.close();
                delete adj_list;
                throw invalid_argument("Incorrect input");
            }
        }
        input.close();
    }

public:

    OrGraph(string filename)
    {
        adj_list = new Adj_List;
        input(filename);
        if ((!adj_list->find("T") || (!adj_list->find("S"))))
            throw invalid_argument("Graph must contain S- and T- elements");

        length = adj_list->get_size();
        E = new int[length];
        H = new int[length];
        int i;
        for (i = 0; i < length; i++)
        {
            E[i] = 0;
            H[i] = 0;
        }
        queue = new List();
    }

    int LTF()
    {
        Init();

        ElemL2* queue_elem = queue->at(0);
        Adj_List_Elem* Cur;
        int temp;
        while (queue_elem)
        {
            Cur = adj_list->at(queue_elem->inf);
            if (DISCHARGE(Cur))
            {
                temp = queue_elem->inf;
                queue->remove(queue->get_index_of(temp));
                queue->push_front(temp, 0);
                queue_elem = queue->at(0);
            }
            queue_elem = queue_elem->next;
        }

        return E[adj_list->get_index_of("T")];
    }

    ~OrGraph()
    {
        delete adj_list;
        delete queue;
        delete[] E;
        delete[] H;
    }
};

int main()
{
    string filename;
    cout << "Enter name of file:\n";
    getline(cin, filename);
    try
    {
        OrGraph G(filename);
        cout << "Max flow is " << G.LTF() << endl;
    }
    catch (exception& e)
    {
        cout << "Error: " << e.what() << endl;
    }
}
