# hw4-Memory-Manager

## **改變POLICY的結果**

因為TLB Replacement Policy為RANDOM的話，每次的結果會不一樣，所以這邊只比較TLB Replacement Policy:LRU時，改變Page Replacement Policy,Frame Allocation Policy的結果。

並且這邊針對process,virtual page,frame數也固定為以下：

Number of Processes: 2	  
Number of Virtual Page: 128  
Number of Physical Frame: 64  

### Page Replacement Policy: FIFO,Frame Allocation Policy: GLOBAL

Process A, Effective Access Time = 164.758  
Process A, Page Fault Rate: 0.723  
Process B, Effective Access Time = 163.709  
Process B, Page Fault Rate: 0.665  

### Page Replacement Policy: FIFO,Frame Allocation Policy: LOCAL

Process A, Effective Access Time = 164.980  
Process A, Page Fault Rate: 0.774  
Process B, Effective Access Time = 163.144  
Process B, Page Fault Rate: 0.700  

### Page Replacement Policy: CLOCK,Frame Allocation Policy: GLOBAL

Process A, Effective Access Time = 164.758  
Process A, Page Fault Rate: 0.723  
Process B, Effective Access Time = 163.709  
Process B, Page Fault Rate: 0.665  

### Page Replacement Policy: CLOCK,Frame Allocation Policy: LOCAL

Process A, Effective Access Time = 164.980  
Process A, Page Fault Rate: 0.774  
Process B, Effective Access Time = 163.522  
Process B, Page Fault Rate: 0.694  
