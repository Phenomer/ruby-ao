#include<stdio.h>
#include<stdlib.h>
#include "cao.h"

static dev_data *devices = NULL;

/*
 * デバイス一覧にデバイスを追加する。
 * 追加した構造体へのポインタを返す。
 * 割当に失敗した場合は-1を返す。
 */
dev_data *
append_device(ao_device *dev, ao_sample_format *format,
	      ao_option *option)
{
  dev_data *behind  = NULL;
  dev_data *current = devices;

  while (current != NULL){
    behind  = current;
    current = current->next;
  }
  current         = ALLOC(dev_data);
  current->device = dev;
  current->format = format;
  current->option = option;
  current->next   = NULL;
  if (behind != NULL){
    behind->next  = current;
  } else {
    devices = current;
  }

  return current;
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
  ao_close(devdat->device);
  ao_free_options(devdat->option);
  free(devdat->format);
  devdat->device = NULL;
  devdat->option = NULL;
  devdat->format = NULL;
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
  
  fprintf(stderr, "rm - dev: %p, fmt: %p, opt: %p\n", 
	  devdat->device,
	  devdat->format,
	  devdat->option);

  /* リストの先頭がNULLではなくかつ先頭にマッチした場合 */
  if (devices != NULL && devices == devdat){
    close_device(devdat);
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
	 それを前のデータのnextに設定する。 */
      if (devdat->next != NULL){
	behind->next = devdat->next;
      }
      close_device(devdat);
      free(devdat);
      return;
    }
    behind  = current;
    current = current->next;
  }
  
  /* バグがなければ到達しない */
  fputs("fatal library error.", stderr);
  exit(1);
  return;
}
