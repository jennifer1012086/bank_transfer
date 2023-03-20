## Bank transfer

編譯與執行:<br>
在Linux 環境使用 Makefile 進行編譯 (gcc version 需大於 7.5.0)

<br>

程式作用:<br>
定義500個 pthread 進行500個銀行帳號的交易，執行 bank 會開始交易，按下 ctl+C 會停止隨機交易，完成剩餘的交易，並印出500個帳號現在的餘額狀況(可能會有負數)

<br>

使用技巧:<br>
1. pthread 創立與整合 <br>
2. signal 處理 <br>
3. lock 與 dead lock 處理 

<br>

正確性證明:<br>
1. Dead lock 避免 <br>
每個 thread 在進入 money transfer 會先進行 id 編號大小的比較，可以避免兩個帳號同時要轉帳給對方，而互相持有一個lock造成死結<br>
2. 互斥執行<br>
由 atomic_compare_exchange_strong 函式同時進行比較與鎖上，一旦鎖上其他人就不能進行轉帳，而需要在各交易中的 while迴圈中進行等待，故不會有一個帳號同時做不同交易的情況發生<br>
3. Progress <br>
lock只要為0就會讓while迴圈 break，兩個lock只要同時被持有就可以進入critical section <br>
4. 程式的完整性 <br>
一旦收到signal則程式會把現有的交易完成，再判斷狀態，因此不會有交易中斷的情況
