library("methylKit")
setwd("/path/to/your/file")
file.list <- list("AlDA1_joinedq_merged_bismark_bt2.CpG_report.txt","AlDA2_joinedq_merged_bismark_bt2.CpG_report.txt", "AlDB1_joinedq_merged_bismark_bt2.CpG_report.txt", "AlDB2_joinedq_merged_bismark_bt2.CpG_report.txt", "AlPA1_joinedq_merged_bismark_bt2.CpG_report.txt", "AlPA2_joinedq_merged_bismark_bt2.CpG_report.txt", "AlPB1_joinedq_merged_bismark_bt2.CpG_report.txt", "AlPB2_joinedq_merged_bismark_bt2.CpG_report.txt")
my_obj <- methRead(file.list, sample.id = list("DA1", "DA2", "DB1", "DB2", "PA1", "PA2", "PB1", "PB2"), assembly = "mock", treatment=c(1,1,1,1,0,0,0,0), context="CpG", pipeline="bismarkCytosineReport")
getMethylationStats(my_obj[[2]],plot=TRUE,both.strands=FALSE)
getCoverageStats(my_obj[[2]],plot=TRUE,both.strands=FALSE)
meth <- unite(my_obj)
getCorrelation(meth,plot=TRUE)
clusterSamples(meth, dist="correlation", method="ward", plot=TRUE)
myDiff <- calculateDiffMeth(meth)
myDiff25p.hyper=getMethylDiff(myDiff,difference=25,qvalue=0.01,type="hyper")
myDiff25p.hypo=getMethylDiff(myDiff,difference=25,qvalue=0.01,type="hypo")
Data25p.hyper <- getData(myDiff25p.hyper)
Data25p.hypo <- getData(myDiff25p.hypo)
numbers25hyperDP <- Data25p.hyper[,2]
numbers25hypoDP <- Data25p.hypo[,2]
write(numbers25hyperDP, file = "numbers25hyperDP", ncolumns = 1)
write(numbers25hypoDP, file = "numbers25hypoDP", ncolumns = 1)

