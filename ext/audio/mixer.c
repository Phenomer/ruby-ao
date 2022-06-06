#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include "cao.h"

int
mix_sample_8(sample_t **samples,  sample_t *ret, int chs){
  int32_t sum;
  int ch;
  uint32_t pos;
  int8_t *buffer;
  int8_t *src;

  buffer = (int8_t *)ret->buffer;
  for (pos=0; pos<ret->bytes; pos++){
    sum = 0;
    for (ch=0; ch<chs; ch++){
      if (pos < samples[ch]->bytes){
	src = (int8_t *)samples[ch]->buffer;
	sum += src[pos];
      }
    }
    buffer[pos] = (int8_t)(sum / chs);
  }
  return 1;
}

int
mix_sample_16(sample_t **samples, sample_t *ret, int chs){
  int32_t sum;
  int ch;
  uint32_t pos;
  int16_t *buffer;
  int16_t *src;

  buffer = (int16_t *)ret->buffer;
  for (pos=0; pos< ret->bytes / 2; pos++){
    sum = 0;
    for (ch=0; ch<chs; ch++){
      if (pos < samples[ch]->bytes / 2){
	src = (int16_t *)samples[ch]->buffer;
	sum += src[pos];
      }
    }
    buffer[pos] = (int16_t)(sum / chs);
  }

  return 1;
}

int
mix_sample(int bits, sample_t **samples,
	   sample_t *ret, int chs){
  if (bits == 8){
    return mix_sample_8(samples, ret, chs);
  } else if(bits == 16){
    return mix_sample_16(samples, ret, chs);
  } else {
    return -1;
  }
}

/*
 * call-seq: Audio::Mixer.mix(samples)
 *
 * 複数のオーディオサンプルをMixする。
 * samplesには、Mix対象のデータ(String)の配列を入れる。
 * 8bitまたは16bitのみ対応
 *
 * [arg1] bits(8 or 16)
 * [arg2] samples([sample1, sample2, ...])
 * [return] Mixed string
 */
VALUE
rao_mix_samples(VALUE obj, VALUE bits, VALUE rsamples){
  VALUE rsample;
  sample_t **samples;
  sample_t ret;
  int stat;
  int chs = 0;
  uint32_t maxlen = 0;

  Check_Type(bits, T_FIXNUM);
  Check_Type(rsamples, T_ARRAY);
  samples = malloc(sizeof(sample_t *) * 1);
  while(1){
    rsample = rb_ary_entry(rsamples, chs);
    if (TYPE(rsample) == T_NIL){
      break;
    }
    Check_Type(rsample, T_STRING);
    samples[chs] = malloc(sizeof(sample_t));
    samples[chs]->buffer = StringValuePtr(rsample);
    samples[chs]->bytes  = RSTRING_LEN(rsample);
    if (samples[chs]->bytes > maxlen){
      maxlen = samples[chs]->bytes;
    }
    chs++;
    samples = realloc(samples, sizeof(sample_t *) * chs);
  }

  ret.bytes  = sizeof(char) * maxlen;
  ret.buffer = malloc(ret.bytes);
  stat = mix_sample(FIX2INT(bits), samples, &ret, chs);
  free(samples);
  if (stat == -1){
    rb_raise(cAO_eAOError, "Unsupported format.");
    return Qnil;
  }
  rsample = rb_str_new(ret.buffer, ret.bytes);
  free(ret.buffer);
  return rsample;
}

void
init_mixer(void)
{
  /*
   * Document-class: Audio::Mixer
   *
   * オーディオデータをシンプルにミックスするモジュール。
   */
  cAO_Mixer = rb_define_module_under(cAudio, "Mixer");

  rb_define_module_function(cAO_Mixer, "mix", rao_mix_samples, 2);
}
