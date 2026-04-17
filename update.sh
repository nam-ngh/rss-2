#!/bin/bash
source ./config.sh
mkdir -p $OUTPUT_DIR/Rx $OUTPUT_DIR/Tx
cp Rx.ino $OUTPUT_DIR/Rx
cp *.h $OUTPUT_DIR/Rx
cp Tx.ino $OUTPUT_DIR/Tx
cp *.h $OUTPUT_DIR/Tx