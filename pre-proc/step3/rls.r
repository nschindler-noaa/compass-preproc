fmat <- read.table("2008biop0328.lwgflowavg.txt", header=FALSE)[1:70,c(1,3)] ## reads LWG part of file, columns 1 and 3

# Note: the <- operator is for assigning values to an object

# Chinook
chstart <- round((134.8840679 - 36)  - 0.1423965*fmat[,2]) #regression equation operates on vector fmat[,2]=avg flow column
chst.out <- data.frame(cbind(as.character(fmat[,1]), chstart), row.names=NULL)	  # unnecessary in final preprocessing
write.table(chst.out, file="ch1.start.out", row.names=FALSE, col.names=FALSE, quote=FALSE) # unnecessary

# Steelhead
sthdstart <- round( (139.8129056 - 44 )  - 0.1194203*fmat[,2])
sthdst.out <- data.frame(cbind(as.character(fmat[,1]), sthdstart), row.names=NULL)
write.table(sthdst.out, file="sthd.start.out", row.names=FALSE, col.names=FALSE, quote=FALSE) 

## Notes
# Regression eqns:  startdate = B0 + B1*avgflow
# Regression parameters:
# Chinook
# B0 = 134.8840679 - 36 = 98.88407
# B1 = -0.1423965

# Steelhead
# B0 = 139.8129056 - 44 = 95.8129
# B1 = - 0.1194203
