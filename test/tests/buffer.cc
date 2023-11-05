#include <fixtures/buffer.h>

#include <nobind.h>

Nobind::Typemap::Buffer nobind_get_buffer() {
  Nobind::Typemap::Buffer buf{nullptr, 16};
  get_buffer(buf.first, buf.second, 0x17);
  return buf;
}
void nobind_put_buffer(Nobind::Typemap::Buffer buf) {
  put_buffer(buf.first, buf.second, 0x17);
}

NOBIND_MODULE(buffer, m) {
  m.def<&nobind_get_buffer>("get_buffer")
  .def<&nobind_put_buffer>("put_buffer");
}
