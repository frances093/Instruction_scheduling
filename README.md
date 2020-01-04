# Instruction_scheduling  
## 簡介：  
* 實作 Tomasulo。
   * 不包含 ROB 。
   * 同個 cycle 可同時執行 issue、dispatch、write back ，但不可對同個 instruction。
   * 這回合釋出的空間（RS 和 ALU）必須等下一個 cycle 才能被使用。
   * Dispatch 的回合也算在 ALU 運算時間內，write back 在運算時間的最後一 cycle 執行。
* 使用語言：C++。

## Input：
* 檔案：（要和程式放在同一層） .txt 檔，裡面包含數個要執行的 instruction  。

        .numofRS:
        add:3
        mul:2

        .Time:
        add:2
        sub:2
        mul:10
        div:40
        addi:2

        .Initial:
        R1=0
        R2=2
        R3=4
        R4=6
        R5=8

        .Code:
        I1:addi	R1, R2, 1
        I2:sub	R1, R3, R4
        I3:div	R1, R2, R3
        I4:mul	R2, R3, R4
        I5:add	R2, R4, R2
        I6:addi	R4, R1, 2
        I7:mul	R5, R5, R5
        I8:add	R1, R4, R4

## Output：  
* 執行視窗：（會印出所有cycle，以下只列舉部分Cycle）

    * 當此 cycle 未進行 issue、dispatch、write back 任一動作時。 

            Cycle: 62

            Do nothing
        

    * 當此 cycle 進行 issue、dispatch、write back 任一動作時

            Cycle: 63

            Write back: I7(=64)

            RS status(add):
                    RS0             /               /
                    RS1             /               /
                    RS2             /               /
            ALU(add): empty

            RS status(mul):
                    RS3             /               /
                    RS4             /               /
            ALU(mul): empty


            RAT status:
                    R0      R1      R2      R3      R4      R5      R6      R7
                    empty   empty   empty   empty   empty   empty   empty   empty

            RF status:
                    R0      R1      R2      R3      R4      R5      R6      R7
                    0       4       30      4       2       64      0       0

* 檔案：（名稱：Ttrack.txt）

            Iss.	Dis.	Wri.
        I1	1	2	3	
        I2	2	4	5	
        I3	3	4	43	
        I4	4	44	53	
        I5	5	54	55	
        I6	6	44	45	
        I7	7	54	63	
        I8	8	46	47   
    

## Step：  
1. 讀取輸入檔案：

        void readfile()

    * 此 funtion 會再呼叫：

            void generateRS(string line)

        用於讀取並產生指定的 RS 數量。

            void assigntimecomsum(string line)

        用於讀取並儲存每個指令在 ALU 中需要多少時間才能得到結果。

            void initialregister(string line)

        用於讀取並儲存檔案中指定的 register 初始值。

            void storeinstruction(string line)

        用於讀取並儲存 instruction 。

2. 將 instruction 轉成 int 形式儲存方便之後執行：

        void transinst()

    * 此 funtion 會再呼叫：

            int findnum(string a)　及　int getnum(string a)
        
        兩者皆會回傳輸入 string 相應的 int 協助轉換（後者專門處理不只一個位數的數字）。

3. 產生 track table，用於最後將每個 instruction 在哪個 cycle 執行 issue、dispatch、write back 輸出成檔案：

        void gettracktable()

4. 產生需要的 ALU 數量：

        void generateALU(int num)

5. 若尚有 instrution 未被 issue 則呼叫：

        bool Issue(int ix)

    此 function 會根據前 issue 的 instruction 的類型檢查相應類型的 RS 是否有空位，有的話則將目前 instruction 放入，更新 track table 並回傳 true ；沒有的話則回傳 false。
    * 此 function 會再呼叫：
        
            bool putinI(int ix, int rsx, RSX *ptrr)        

        用於確定是否有空並將目前 instruction 放入空的 RS 中。
            
            
        * 此 function 會再呼叫：

                void datacapture(RSX *ptrr)

            基於剛放入的 instruction 的 operants 所表示的是其所在的 RF 位置，此 function 是用於將其轉換成 RF 中的 data 或目前等待的 RS 編號。 
            
6. 若 RS 中尚有 instruction 則：
    檢查相應的 ALU 是否已被佔據，否則回傳 -1 。是，則：
    找到可dispatch 的 instruction，回傳此 instruction 的編號，並更新 track table。若沒有可 dispatch 的，則回傳 -1：

        int Dispatchadd(ALU *ptra, int nowissue)　或　
        int Dispatchmul(ALU *ptra, int nowissue)

    * 此 function 會再呼叫：

            int forfirstin(RSX *ptrr, int nowissue)
        
        用於找出目前 operant 皆準備好，且是準備好的 instruction 中最早進入的。

            int putinD(RSX *ptrr, int rsx, ALU *ptra)

        用於將選到的 instruction 放入 ALU 中，並設定何時會完成計算（release），並回傳放入的 instruction 編號。

7. 檢查 ALU 是否被佔據。否，則回傳 false 。是，則：  
     檢查目前 cycle 是否等於 release。否，則回傳 false 。是，則：  
    計算此 instruction 的 outcome ，並將結果更新到 RAT、RF、RS 中等待此 data 的 instruction （如果需要），並更新 track table，並回傳 true：

        bool Writeback(ALU *ptra)

    * 此 function 會再呼叫：

            int getoutcome(int type, int op1, int op2)

        用於得到此 instruction 的運算結果，並回傳。
        
            void releaseRS(RSX *ptrr)
            
        用於釋出此 instruction 佔用的 RS 。

            void broadcast(RSX *ptrr, int ix, int outcome)

        用於更新 RS 中，等待此 instruction 結果的 instruction。

8. 如果 5. 或 6. 或 7. 中有任一步驟有移動 instruction 則 print 出目前 RS、ALU、RAT 及 RF 的狀態。無，則 print 出 "Do nothing"：
        
        void printstatus()

    * 此 function 會再呼叫：

              void printALU(ALU *ptra)

        用於 print 出 ALU 狀態。


            void printRS(int rsx, RSX *ptrr)

        用於 print 出 RS 狀態。
                    
        * 此 function 會再呼叫：
        
                string getsign(int x)

            用於協助 print 出 RS 中，不同 instruction 的表示符號。 


9. 重複執行 5. ~ 8. 直到所有 instruction 皆被 issue 過，且 RS 及 ALU 皆完全空出為止。
    * 此過程會呼叫：

            bool isempty(ALU *ptra)

        用於確定 ALU 是否被佔據。

10. 將 track table 輸出成檔案：

        void outputtracktable()        

    * 在 5. ~ 8. 中，若有移動過 instruction 就會更新 track table ，此動作藉由以下 function 完成：
        
            void updatetracktable(int type, int ix)
        

11. 執行結束。
