# TCP


Parna Asadi - 810198498
Morteza Bahjat - 810198498


# Part One


- - What is different between Selective Repeat and Go Back N?


- Both Go-Back-N Protocol and Selective Repeat Protocol are the types of sliding window protocols. The main difference between these two protocols is that after finding the suspect or damage in sent frames go-back-n protocol re-transmits all the frames whereas selective repeat protocol re-transmits only that frame which is damaged.


#### Go Back N :


Go-Back-N protocol is a sliding window protocol. It is a mechanism to detect and control the error in datalink layer. During transmission of frames between sender and receiver, if a frame is damaged, lost, or an acknowledgement is lost then the action performed by sender and receiver is explained in the following content.

![](https://techdifferences.com/wp-content/uploads/2016/05/Go-Back-N.jpg)


- Sender window size of Go-Back-N Protocol is N.
- Receiver window size of Go-Back-N Protocol is 1.
- Go-Back-N Protocol is less complex.
- In Go-Back-N Protocol, Out-of-Order packets are NOT Accepted (discarded) and the entire window is re-transmitted.
- In Go-Back-N Protocol, neither sender nor at receiver need sorting.
- If error rate is high, it wastes a lot of bandwidth.
- No searching of frame is required neither on sender side nor on receiver.
- It more often used.



#### Selective Repeat


Selective repeat is also the sliding window protocol which detects or corrects the error occurred in the datalink layer. The selective repeat protocol retransmits only that frame which is damaged or lost. In selective repeat protocol, the retransmitted framed is received out of sequence. 

![](https://techdifferences.com/wp-content/uploads/2016/05/Selective-repeat.jpg)

- Sender window size of selective Repeat protocol is also N.
- Receiver window size of selective Repeat protocol is N.
- In selective Repeat protocol, receiver side needs sorting to sort the frames.
- In selective Repeat protocol, Out-of-Order packets are Accepted.
- More complex as it require to apply extra logic and sorting and storage, at sender and receiver.
- Comparatively less bandwidth is wasted in retransmitting.
- The sender must be able to search and select only the requested frame.
- It is less in practice because of its complexity.


<hr/>

- - Report Selective Repeat protocol
