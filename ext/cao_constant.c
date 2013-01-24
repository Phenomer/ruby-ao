#include "cao.h"

VALUE cAudio;
VALUE cAO;

/* constants */
void
init_constant(void)
{
  /* ドライバがLive出力用であることを示す。 */
  rb_define_const(cAO, "TYPE_LIVE",  INT2FIX(AO_TYPE_LIVE));

  /* ドライバがファイル出力用であることを示す。 */
  rb_define_const(cAO, "TYPE_FILE",  INT2FIX(AO_TYPE_FILE));

  /* データのエンディアンがリトルエンディアンであることを示す。 */
  rb_define_const(cAO, "FMT_LITTLE", INT2FIX(AO_FMT_LITTLE));

  /* データのエンディアンがビッグエンディアンであることを示す。 */
  rb_define_const(cAO, "FMT_BIG",    INT2FIX(AO_FMT_BIG));

  /* データのエンディアンがホストのネイティブ形式であることを示す。 */
  rb_define_const(cAO, "FMT_NATIVE", INT2FIX(AO_FMT_NATIVE));
  return;
}
