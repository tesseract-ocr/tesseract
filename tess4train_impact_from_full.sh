#!/bin/bash
################################################################
# Simple bash script to run 'Impact from Full' Finetuning
# as described in tesstutorial wiki pages.
################################################################
## Language Parameters
LANG=eng
NORM_MODE=1
#
## Only traineddata files from tessdata_best can be used for training.
BEST_TRAINEDDATA="$HOME/tessdata_best/$LANG.traineddata"
#
## Script unicharsets, wordlists etc are used from langdata repo.
LANGDATA_DIR=$HOME/langdata
#
## Directory which has the box/tiff pairs to be used for training
## Please ensure that they are in format needed for LSTM Training.
BOXTIFF_DIR=$HOME/TEST/engtrain
#
## Output directory which will have the work files as well as
## the finetuned traineddata file.
OUTPUT_DIR=$HOME/tesstutorial/eng
#
#################################################################

rm -rf $OUTPUT_DIR
mkdir $OUTPUT_DIR
cd $OUTPUT_DIR
#
cp $BOXTIFF_DIR/*.box $OUTPUT_DIR/
cp $BOXTIFF_DIR/*.tif $OUTPUT_DIR/
#
combine_tessdata -e $BEST_TRAINEDDATA $OUTPUT_DIR/$LANG.lstm
#
my_box_files=$(ls *.box)
my_tiff_files=$(ls *.tif)

unicharset_extractor \
--output_unicharset $LANG.unicharset \
--norm_mode $NORM_MODE \
$my_box_files

set_unicharset_properties \
-U $LANG.unicharset -O $LANG.unicharset \
-X $LANG.xheights \
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
  --train_listfile "$OUTPUT_DIR/$LANG.training_files.txt" \
  --max_iterations 400

lstmtraining \
  --stop_training \
  --continue_from  $OUTPUT_DIR/impact_checkpoint \
  --traineddata $BEST_TRAINEDDATA \
  --model_output $OUTPUT_DIR/$LANG-impact.traineddata


  
