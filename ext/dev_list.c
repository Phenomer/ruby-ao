#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include "cao.h"

static dev_data *devices = NULL;

void
free_sample_buffer(sample_buffer *head){
  sample_buffer *current = head;
  sample_buffer *behind;

  while(current != NULL){
    behind  = current;
    current = current->next;
    free(behind);
  }
  return;
}

/*
 * デバイス一覧にデバイスを追加する。
 * 追加した構造体へのポインタを返す。
 * 割当に失敗した場合は-1を返す。
 */
dev_data *
append_device(ao_device *dev, ao_sample_format *format,
	      ao_option *option)
{
  dev_data *current = devices;
  dev_data *behind  = NULL;
  dev_data *newdev  = NULL;

  while (current != NULL){
    behind  = current;
    current = current->next;
  }
  newdev          = ALLOC(dev_data);
  newdev->device  = dev;
  newdev->format  = format;
  newdev->option  = option;
  newdev->playing = 0;
  newdev->buffer  = NULL;
  newdev->next    = NULL;
  if (behind != NULL){
    behind->next  = newdev;
  } else {
    devices = newdev;
  }

  return newdev;
}

/*
 * devdata構造体の持つデバイスを全て閉じ、
 * またオプションとフォーマットの情報も削除する。
 * メンバ変数にはNULLを設定する。
 * devdata構造体自体は開放しない。
 * (rubyのGCによって実行されるremove_device関数に任せるため)
 */
void
close_device(dev_data *devdat){
<<<<<<< local
  if (devdat->playing < 0){
    return;
  }
  devdat->playing = -1;
  pthread_join(devdat->thread, NULL);
  ao_close(devdat->device);
  ao_free_options(devdat->option);
  free_sample_buffer(devdat->buffer);
  free(devdat->format);
=======
  if (devdat->device != NULL){
    ao_close(devdat->device);
  }
  if (devdat->option != NULL){
    ao_free_options(devdat->option);
  }
  if (devdat->format != NULL){
    free(devdat->format);
  }
>>>>>>> other
  devdat->device = NULL;
  devdat->option = NULL;
  devdat->format = NULL;
  devdat->buffer = NULL;
  return;
}

/* 
 * デバイス一覧からdev_data構造体へのポインタを元にデバイスを削除する。
 * この関数はopen_live()とopen_file()にてdev_data構造体をWrapする時に
 * 構造体free用関数として渡す(rubyのGCにて呼ばれるよう設定する)
 * Data_Wrap_Struct(cAO_cDeviceData, 0, remove_device, devdat);
 */
void
remove_device(dev_data *devdat)
{
  dev_data *behind  = NULL;
  dev_data *current = devices;

  close_device(devdat);

  /* 先頭にマッチした場合 */
  if (devices == devdat){
    /* 次のデータが存在している場合、先頭をそのデータに入れ替える
     存在しない場合先頭をNULLにする */
    if (devices->next != NULL){
      devices = devdat->next;
    } else{
      devices = NULL;
    }
    free(devdat);
    return;
  }

  /* リスト先頭がマッチしなかった場合 */
  while (current != NULL){
    if (current == devdat){
      /* 削除するデータの次にデータが存在すれば、
	 それを前のデータのnextに設定する。
	 存在しなければNULLを設定する。 */
      if (devdat->next != NULL){
	behind->next = devdat->next;
      } else {
	behind->next = NULL;
      }
      free(devdat);
      return;
    }
    behind  = current;
    current = current->next;
  }
  /* バグがなければ到達しない */
  fputs("fatal ao library error.\n", stderr);
  exit(1);
  return;
}
