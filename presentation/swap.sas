int swap(int* p1, int* p2)
{
    int tmp = *p2
    *p2 = *p1 
    *p1 = tmp
}

int main()
{
    int x1 = 3
    int x2 = 4

    printI(x1)
    printI(x2)

    swap(&x1, &x2)

    printI(x1)
    printI(x2)
}