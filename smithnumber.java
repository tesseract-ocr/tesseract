import java.io.*;
class Smith{
    public static void main(String args[])
    throws IOException{
        InputStreamReader in = new InputStreamReader(System.in);
        BufferedReader br = new BufferedReader(in);
        System.out.print("N = ");
        int n = Integer.parseInt(br.readLine());
        if(isComposite(n)){
            int sum1 = sumOfDigits(n);
            int sum2 = sumOfDigitsOfPrimeFactors(n);
            if(sum1 == sum2)
                System.out.println(n + " is a Smith Number.");
            else
                System.out.println(n + " is not a Smith Number.");
        }
        else
            System.out.println(n + " is not a Smith Number.");
    }
    public static boolean isComposite(int n){
        int f = 0;
        for(int i = 1; i <= n; i++)
            if(n % i == 0)
                f++;
        if(f > 2)
            return true;
        return false;
    }
    public static int sumOfDigits(int n){
        int s = 0;
        for(int i = n; i != 0; i /= 10)
            s += i % 10;
        return s;
    }
    public static int sumOfDigitsOfPrimeFactors(int n){
        int s = 0;
        int f = 2;
        while(n != 1){
            while(n % f == 0 && f <= n){
                s += sumOfDigits(f);
                n /= f;
            }
            f++;
        }
        return s;
    }
}
