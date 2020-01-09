./bismark_genome_preparation ./genome
./bismark --non_directional --genome ./genome ./AlDA1_joinedq_merged.fq 
./bismark --non_directional --genome ./genome ./AlDA2_joinedq_merged.fq 
./bismark --non_directional --genome ./genome ./AlDB1_joinedq_merged.fq 
./bismark --non_directional --genome ./genome ./AlDB2_joinedq_merged.fq 
./bismark --non_directional --genome ./genome ./AlPA1_joinedq_merged.fq 
./bismark --non_directional --genome ./genome ./AlPA2_joinedq_merged.fq 
./bismark --non_directional --genome ./genome ./AlPB1_joinedq_merged.fq 
./bismark --non_directional --genome ./genome ./AlPB2_joinedq_merged.fq 
./bismark_methylation_extractor --bedGraph --cytosine_report --genome_folder ./genome AlDA1_joinedq_merged_bismark_bt2.bam
./bismark_methylation_extractor --bedGraph --cytosine_report --genome_folder ./genome AlDA2_joinedq_merged_bismark_bt2.bam
./bismark_methylation_extractor --bedGraph --cytosine_report --genome_folder ./genome AlDB1_joinedq_merged_bismark_bt2.bam
./bismark_methylation_extractor --bedGraph --cytosine_report --genome_folder ./genome AlDB2_joinedq_merged_bismark_bt2.bam
./bismark_methylation_extractor --bedGraph --cytosine_report --genome_folder ./genome AlPA1_joinedq_merged_bismark_bt2.bam
./bismark_methylation_extractor --bedGraph --cytosine_report --genome_folder ./genome AlPA2_joinedq_merged_bismark_bt2.bam
./bismark_methylation_extractor --bedGraph --cytosine_report --genome_folder ./genome AlPB1_joinedq_merged_bismark_bt2.bam
./bismark_methylation_extractor --bedGraph --cytosine_report --genome_folder ./genome AlPB2_joinedq_merged_bismark_bt2.bam
