#!/bin/bash
################################################################
# Simple bash script to run 'Impact from Full' Finetuning
# as described in tesstutorial wiki pages but using
# user specified box/tiff pairs rather than synthetic training
# data created by text2image using fonts and utf8 training text.
################################################################
# Set to yes for first time run to setup directories and download required files.
FirstTimeSetup=no
# Set to yes if you already have matching box files suitable for LSTM training.
CopyBox=no
# Set to yes if you need to create box files from tif images. These are in 
# WordStr format per line and can be easily edited to match ground truth.
GenerateBox=no
################################################################
## Language Parameters
LANG=eng
# General Rule: Set to 1 for languages in Latin script, Set to 2 for others.
NORM_MODE=1
#
## Directory with custom box/tiff pairs to be used for training
## Please ensure that they are in format needed for LSTM Training.
## CHANGE to your directory path.
BOXTIFF_DIR=$HOME/TEST/engtrain
#
## Output directory which will have the work files as well as
## the finetuned traineddata file.
## CHANGE to your directory path.
OUTPUT_DIR=$HOME/tesstutorial/$LANG-boxtiff
#
## Only traineddata files from tessdata_best can be used for training.
BEST_TRAINEDDATA=$OUTPUT_DIR/tessdata_best/$LANG.traineddata
#
## Script unicharsets, wordlists etc are used from langdata repo.
LANGDATA_DIR=$OUTPUT_DIR/langdata
#
my_tiff_files=$(ls $OUTPUT_DIR/*.tif)
my_box_files=$(ls $OUTPUT_DIR/*.box)
#################################################################
if [ $FirstTimeSetup = "yes" ]; 
then
    rm -rf $OUTPUT_DIR
    mkdir $OUTPUT_DIR
    mkdir $OUTPUT_DIR/tessdata_best
    mkdir $LANGDATA_DIR

    wget -O $BEST_TRAINEDDATA https://github.com/tesseract-ocr/tessdata_best/raw/master/$LANG.traineddata
    wget -O $LANGDATA_DIR/Latin.unicharset https://raw.githubusercontent.com/tesseract-ocr/langdata_lstm/master/Latin.unicharset
    wget -O $LANGDATA_DIR/radical-stroke.txt https://raw.githubusercontent.com/tesseract-ocr/langdata_lstm/master/radical-stroke.txt
    wget -O $OUTPUT_DIR/lstm.train https://raw.githubusercontent.com/tesseract-ocr/tesseract/master/tessdata/configs/lstm.train

    combine_tessdata -e $BEST_TRAINEDDATA $OUTPUT_DIR/$LANG.lstm

    cp $BOXTIFF_DIR/*.tif $OUTPUT_DIR/
fi
###
if [ $CopyBox = "yes" ]; 
then
    cp $BOXTIFF_DIR/*.box $OUTPUT_DIR/
else
    if [ $GenerateBox = "yes" ]; 
    then
        for my_file in ${my_tiff_files}; do
            tesseract $my_file "${my_file%.*}" --oem 1 --psm 6 -l $LANG --tessdata-dir $OUTPUT_DIR/tessdata_best -c tessedit_create_wordstrbox=1
        done
        echo -e "***** The Boxfiles need to be reviewed and corrected as needed."
        echo -e "***** Following steps should be run after that."
        exit 0
    fi
fi
###########################################################
rm $OUTPUT_DIR/*traineddata
rm $OUTPUT_DIR/*checkpoint

unicharset_extractor \
--output_unicharset $OUTPUT_DIR/$LANG.unicharset \
--norm_mode $NORM_MODE \
$my_box_files

set_unicharset_properties \
-U $OUTPUT_DIR/$LANG.unicharset -O $OUTPUT_DIR/$LANG.unicharset \
-X $OUTPUT_DIR/$LANG.xheights \
--script_dir=$LANGDATA_DIR

for my_file in ${my_tiff_files}; do
    tesseract $my_file "${my_file%.*}" --psm 6 lstm.train 
done

ls -1 "$OUTPUT_DIR/$LANG".*.lstmf > "$OUTPUT_DIR/$LANG.training_files.txt"

lstmtraining \
  --debug_interval  0 \
  --model_output $OUTPUT_DIR/impact \
  --continue_from $OUTPUT_DIR/$LANG.lstm \
  --traineddata $BEST_TRAINEDDATA \
  --train_listfile $OUTPUT_DIR/$LANG.training_files.txt \
  --max_iterations 400

lstmtraining \
  --stop_training \
  --continue_from  $OUTPUT_DIR/impact_checkpoint \
  --traineddata $BEST_TRAINEDDATA \
  --model_output $OUTPUT_DIR/$LANG-impact.traineddata

echo -e "Finetuned traineddata is ready - $OUTPUT_DIR/$LANG-impact.traineddata"
  
lstmeval \
 --model $OUTPUT_DIR/$LANG-impact.traineddata \
  --verbosity 0 \
  --eval_listfile  $OUTPUT_DIR/$LANG.training_files.txt

lstmeval \
 --model $BEST_TRAINEDDATA \
  --verbosity 0 \
  --eval_listfile  $OUTPUT_DIR/$LANG.training_files.txt
