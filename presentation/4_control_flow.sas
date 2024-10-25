
int main() 
{
    int num = scanI()
    if(num > 6) 
    {
        while(num > 0)
        {
            printI(1)
            num = num - 1
        }
    }
    else if(num > 0)
    {
        printI(num)
    }
    else
    {
        printI(0)
    }
}