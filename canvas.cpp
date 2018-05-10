#include <stdio.h>
#include <stack>
#include <map>
#include <string>
#include "agg_basics.h"
#include "agg_scanline_u.h"
#include "agg_scanline_p.h"
#include "agg_pixfmt_rgb.h"
#include "agg_pixfmt_rgba.h"
#include "agg_pixfmt_gray.h"
#include "agg_alpha_mask_u8.h"
#include "agg_conv_curve.h"
#include "agg_conv_stroke.h"
#include "agg_path_storage.h"
#include "agg_rendering_buffer.h"
#include "platform/agg_platform_support.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_renderer_scanline.h"
#include "agg_vcgen_markers_term.h"
#include "agg_rounded_rect.h"
#include "agg_trans_affine.h"
#include "agg_arc.h"


namespace canvas {

typedef agg::pixfmt_rgba32 rgba32;
typedef agg::rgba Color;

struct ContextStackItem
{
  double m_miter_limit;
  int32_t m_width;
  agg::line_cap_e m_cap;
  agg::line_join_e m_join;
  Color m_fill_color;
  Color m_stroke_color;
  agg::trans_affine m_mtx;
};

class Canvas
{
public:
  Canvas(int32_t w, int32_t h, int32_t c, uint8_t* buffer);
  ~Canvas();

  void save();
  void restore();

  void reset();
  void clear(double r, double g, double b, double a);

  //line、styles、shadow
  void set_line_cap(const char* val);
  agg::line_cap_e  get_line_cap();
  void set_line_join(const char* val);
  agg::line_join_e get_line_join();
  void set_line_width(double val);
  double get_line_width();
  void set_miter_limit(double val);
  double get_miter_limit();
  void set_fill_color(double r, double g, double b, double a);
  Color get_fill_color();
  void set_stroke_color(double r, double g, double b, double a);
  Color get_stroke_color();

  //path relative
  void begin_path();
  void close_path();
  void fill();
  void stroke();
  void move_to(double x, double y);
  void line_to(double x, double y);
  void clip();
  void arc(double x, double y, double r, double start, double end, bool ccw);
  void arc_to(double x1, double y1, double x2, double y2, double r);
  void quadratic_curve_to(double cp1x, double cp1y, double x, double y);
  void bezier_curve_to(double cp1x, double cp1y, double cp2x, double cp2y, double x, double y);
  void is_point_in_path();

  //transform
  void translate(double x, double y);
  void scale(double x, double y);
  void rotate(double val);
  void transform(double a, double b, double c, double d, double e, double f);
  void setTransform(double a, double b, double c, double d, double e, double f);

  //rect
  void rect(double x, double y, double w, double h, double r);
  void fill_rect(double x, double y, double w, double h);
  void stroke_rect(double x, double y, double w, double h);
  void clear_rect(double x, double y, double w, double h);

private:
  double m_miter_limit;
  int32_t m_width;
  int32_t m_height;
  uint32_t m_line_width;
  agg::line_cap_e m_cap;
  agg::line_join_e m_join;
  Color m_fill_color;
  Color m_stroke_color;

  std::stack<ContextStackItem> m_ctx_stacks;

  agg::trans_affine m_mtx;
  agg::path_storage m_path_storage;
  agg::rasterizer_scanline_aa<> m_ras;
  agg::scanline_u8 m_sl;

  agg::pixfmt_rgba32 m_pix;
  agg::renderer_base<agg::pixfmt_rgba32> m_render_base;
  agg::renderer_scanline_aa_solid<agg::renderer_base<agg::pixfmt_rgba32> > m_render;
  // agg::renderer_scanline_bin_solid<agg::renderer_base<Color>> m_render_bin;

  void *m_data; // buffer data

  agg::rendering_buffer m_buffer;

  typedef std::map<std::string, agg::line_cap_e> LineCap;
  typedef std::map<std::string, agg::line_join_e> LineJoin;

  static LineCap m_line_cap_map;
  static LineJoin m_line_join_map;

  static LineCap init_line_cap() 
  {
    LineCap cap;
    cap["butt"]   = agg::line_cap_e::butt_cap;
    cap["round"]  = agg::line_cap_e::round_cap;
    cap["square"] = agg::line_cap_e::square_cap;
    return cap;
  }

  static LineJoin init_line_join()
  {
    LineJoin join;
    join["bevel"] = agg::line_join_e::bevel_join;
    join["round"] = agg::line_join_e::round_join;
    join["miter"] = agg::line_join_e::miter_join;

    return join;
  }
};

Canvas::LineCap Canvas::m_line_cap_map = init_line_cap();
Canvas::LineJoin Canvas::m_line_join_map = init_line_join();

Canvas::Canvas(int32_t w, int32_t h, int32_t c, uint8_t* buffer):
  m_cap(agg::line_cap_e::butt_cap),
  m_join(agg::line_join_e::miter_join),
  m_width(1.0),
  m_height(1.0),  
  m_line_width(1.0),
  m_miter_limit(10.0),
  m_fill_color(Color(100, 0, 0, 1)),
  m_stroke_color(Color(100, 0, 0, 1)) 
{
  m_width = w;
  m_height = h;

  // m_sl = agg::scanline_u8();
  // m_ras = agg::rasterizer_scanline_aa<>();

  m_data = buffer;
  m_buffer = agg::rendering_buffer(buffer, w, h, w * c);
  m_pix = agg::pixfmt_rgba32(m_buffer);
  m_render_base = agg::renderer_base<agg::pixfmt_rgba32>(m_pix);
  m_render = agg::renderer_scanline_aa_solid<agg::renderer_base<agg::pixfmt_rgba32> >(m_render_base);
}

Canvas::~Canvas(void)
{
  //TODO
  // if(m_sl != NULL)
  // {
  //   delete m_sl
  // }
  // if(m_ras != NULL)
  // {
  //   delete m_ras
  // }
}

//The current transformation matrix.
//The current clipping region.
//The current dash list.
//The current values of the following attributes: 
//  strokeStyle, 
//  fillStyle, 
//  globalAlpha, 
//  lineWidth, 
//  lineCap, 
//  lineJoin, 
//  miterLimit, 
//  lineDashOffset, 
//  shadowOffsetX, 
//  shadowOffsetY, 
//  shadowBlur, 
//  shadowColor, 
//  globalCompositeOperation, 
//  font, 
//  textAlign, 
//  textBaseline, 
//  direction, 
//  imageSmoothingEnabled.
void Canvas::save(void)
{
  ContextStackItem ctx;

  ctx.m_width = m_width;  
  ctx.m_cap = m_cap;
  ctx.m_join = m_join;
  ctx.m_miter_limit = m_miter_limit;
  ctx.m_fill_color = m_fill_color;
  ctx.m_stroke_color = m_stroke_color;

  ctx.m_mtx = m_mtx;

  m_ctx_stacks.push(ctx);

  return;
}

void Canvas::restore(void)
{
  if (!m_ctx_stacks.empty())
  {
    ContextStackItem ctx = m_ctx_stacks.top();

    m_cap = ctx.m_cap;
    m_join = ctx.m_join;
    m_width = ctx.m_width;
    m_miter_limit = ctx.m_miter_limit;
    m_fill_color = ctx.m_fill_color;
    m_stroke_color = ctx.m_stroke_color;

    m_mtx = ctx.m_mtx;
    m_ctx_stacks.pop();
  }

  return;
}

agg::line_cap_e Canvas::get_line_cap(void)
{
  return m_cap;
}

void Canvas::set_line_cap(const char* val)
{ 
  if (m_line_cap_map.count(val)) 
  {
    m_cap = m_line_cap_map[val];
  }
  else
  {
    m_cap = agg::line_cap_e::butt_cap;
  }

  return;
}

agg::line_join_e Canvas::get_line_join()
{
  return m_join;
}

void Canvas::set_line_join(const char* val)
{
  if (m_line_join_map.count(val))
  {
    m_join = m_line_join_map[val];
  }
  else
  {
    m_join = agg::line_join_e::miter_join;
  }

  return;
}

double Canvas::get_line_width(void)
{
  return m_line_width;
}

void Canvas::set_line_width(double val)
{
  m_line_width = val;
}

void Canvas::set_miter_limit(double val)
{
  m_miter_limit = val;
}

double Canvas::get_miter_limit()
{
  return m_miter_limit;
}

void Canvas::set_fill_color(double r, double g, double b, double a)
{
  m_fill_color = Color(r, g, b, a);
}

Color Canvas::get_fill_color()
{
  return m_fill_color;
}

void Canvas::set_stroke_color(double r, double g, double b, double a)
{
  m_stroke_color = Color(r, g, b, a);
}

Color Canvas::get_stroke_color()
{
  return m_stroke_color;
}

void Canvas::begin_path()
{
  m_path_storage.start_new_path();
}

void Canvas::close_path()
{
  m_path_storage.close_polygon();
}

void Canvas::move_to(double x, double y)
{
  m_path_storage.move_to(x, y);
}

void Canvas::line_to(double x, double y)
{
  m_path_storage.line_to(x, y);
}

void Canvas::clear(double r, double g, double b, double a = 1.0)
{
  m_render_base.clear(Color(r, g, b, a));
}

void Canvas::reset()
{
  m_mtx.reset();
  m_ras.reset();
  m_ras.reset_clipping();
  m_path_storage.remove_all();
}

void Canvas::stroke()
{
  m_render.color(m_stroke_color);
  agg::conv_curve<agg::path_storage> c(m_path_storage);
  agg::conv_stroke<agg::conv_curve<agg::path_storage> > stroke(c);
  stroke.line_join(m_join);
  stroke.line_cap(m_cap);
  stroke.width(m_line_width);
  agg::conv_transform<agg::conv_stroke<agg::conv_curve<agg::path_storage> >, agg::trans_affine > trans(stroke, m_mtx);
  m_ras.add_path(trans);
  agg::render_scanlines(m_ras, m_sl, m_render);

  return reset();
}

void Canvas::fill()
{
  m_render.color(m_fill_color);
  agg::conv_curve<agg::path_storage> path(m_path_storage);
  agg::conv_transform<agg::conv_curve<agg::path_storage>, agg::trans_affine > trans(path, m_mtx);
  m_ras.add_path(trans);
  agg::render_scanlines(m_ras, m_sl, m_render);

  return reset();
}

void Canvas::clip()
{
  agg::scanline_u8 sl;  
  typedef agg::renderer_base<agg::pixfmt_rgba32> ren_base;
  typedef agg::renderer_scanline_aa_solid<ren_base> renderer;
  printf("m_width=%d\n, m_width=%d\n", m_width, m_height);
  unsigned char* alpha_buf = new unsigned char[m_width * m_height * 4];

  agg::rendering_buffer alpha_mask_rbuf;
  alpha_mask_rbuf.attach(alpha_buf, m_width, m_height, m_width*4);
  agg::pixfmt_rgba32 pixf(alpha_mask_rbuf);
  ren_base rb(pixf);
  renderer r(rb);

  rb.clear(agg::rgba8(1, 1, 1));

  m_ras.add_path(m_path_storage);
  agg::render_scanlines(m_ras, sl, r);

  return;
}

void Canvas::arc(double x, double y, double r, double start, double end, bool ccw)
{
  agg::arc a(x, y, r, r, start, end, ccw);
  m_path_storage.concat_path(a);
}

void Canvas::arc_to(double x1, double y1, double x2, double y2, double r)
{
  //TODO
  // m_path_storage.arc_to()
}

void Canvas::quadratic_curve_to(double cp1x, double cp1y, double x, double y)
{
  m_path_storage.curve3(cp1x, cp1y, x, y);
}

void Canvas::bezier_curve_to(double cp1x, double cp1y, double cp2x, double cp2y, double x, double y)
{
  m_path_storage.curve4(cp1x, cp1y, cp2x, cp2y, x, y);
}

void Canvas::is_point_in_path()
{
  //TODO
}

void Canvas::translate(double x, double y)
{
  m_mtx *= agg::trans_affine_translation(x, y);
}

void Canvas::scale(double x, double y)
{
  m_mtx *= agg::trans_affine_scaling(x, y);
}

void Canvas::rotate(double angle)
{
  m_mtx *= agg::trans_affine_rotation(angle + agg::pi);
}

void Canvas::transform(double a, double b, double c, double d, double e, double f)
{
  agg::trans_affine tmp_mtx(a, b, c, d, e, f);
  m_mtx.multiply(tmp_mtx);
}

void Canvas::setTransform(double a, double b, double c, double d, double e, double f)
{
  m_mtx = agg::trans_affine(a, b, c, d, e, f);
}

void Canvas::rect(double x, double y, double w, double h, double r)
{
  agg::rounded_rect shape(x, y, x + w, y + h, r);
  m_path_storage.concat_path(shape);
}

void Canvas::fill_rect(double x, double y, double w, double h)
{
  agg::rounded_rect shape(x, y, x + w, y + h, 0);
  m_path_storage.concat_path(shape);
  fill();
}

void Canvas::stroke_rect(double x, double y, double w, double h)
{
  agg::rounded_rect shape(x, y, x + w, y + h, 0);
  m_path_storage.concat_path(shape);
  stroke();
}

void Canvas::clear_rect(double x, double y, double w, double h)
{
  Color c = m_fill_color;
  m_fill_color = Color(1, 1, 1);
  fill_rect(x, y, w, h);
  m_fill_color = c;
}

}

class the_application: public agg::platform_support
{
public:
  the_application(agg::pix_format_e format, bool flip_y) :
    agg::platform_support(format, flip_y)
  {
  }

  void draw_line(canvas::Canvas* context)
  {
    context->begin_path();
    context->set_line_width(8);
    context->set_stroke_color(0.1, 0.1, 0.5, 0.8);
    context->move_to(30, 30);
    context->set_line_cap("round");
    context->line_to(100, 100);
    context->stroke();

    context->begin_path();
    context->set_line_width(4);
    context->move_to(230, 30);
    context->line_to(230, 100);
    context->line_to(250, 100);
    context->close_path();
    context->stroke();

    context->begin_path();
    context->move_to(320, 320);
    context->quadratic_curve_to(220,150,300,120);
    context->stroke();

    context->begin_path();
    context->move_to(20,20);
    context->bezier_curve_to(20,100,200,100,200,20);
    context->stroke();

    context->begin_path();
    context->arc(400, 240, 50, 0, 2*3.1415926, true);
    context->stroke();
  }

  void draw_rect(canvas::Canvas* context)
  {
    context->set_line_width(2);
    context->rect(20, 20, 50, 50, 3);
    context->stroke();  
    context->fill_rect(220, 220, 60, 60);
    context->stroke_rect(30, 100, 60, 60);
    context->clear_rect(230, 230, 40, 40);

    // Clip a rectangular area
    context->rect(50,20,200,120, 0);
    context->stroke();
    context->clip();
    // Draw red rectangle after clip()
    context->fill_rect(0,0,150,100);
  }

  void draw_polygons(canvas::Canvas* context)
  {
    //
  }

  void draw_transform(canvas::Canvas* context)
  {
    context->stroke_rect(450,5,25,15);
    context->scale(2,2);
    context->stroke_rect(450,5,25,15);
  }

  virtual void on_draw()
  {
    agg::rendering_buffer &rbuf = rbuf_window();
    printf("stride: %d\n", rbuf.stride());
    printf("width: %d\n", rbuf.width());
    printf("height: %d\n", rbuf.height());
    canvas::Canvas* context = new canvas::Canvas((int32_t)rbuf.width(), (int32_t)rbuf.height(), 4, rbuf.buf());

    context->clear(1, 1, 1);
    draw_line(context);
    draw_rect(context);
    // draw_polygons(context);
    draw_transform(context);

    return;
  }

  virtual void on_drawx()
  {
    agg::rendering_buffer &rbuf = rbuf_window();
    agg::pixfmt_bgr24 pixf(rbuf);

    typedef agg::renderer_base<agg::pixfmt_bgr24> renderer_base_type;
    renderer_base_type renb(pixf);

    agg::rasterizer_scanline_aa<> ras;
    agg::scanline_u8 sl;

    renb.clear(agg::rgba(1, 1, 1));

    agg::path_storage path;

    path.start_new_path();

    //绘制矩形
    // ras.reset();
    agg::rounded_rect r(10, 10, 100, 100, 4);
    // path.concat_path(r);
    ras.add_path(r);
    agg::render_scanlines_aa_solid(ras, sl, renb, agg::rgba(120, 120, 0));
  }
};

int agg_main(int argc, char* argv[])
{
  // the_application app(agg::pix_format_bgr24, false);
  the_application app(agg::pix_format_rgba32, false);
  app.caption("AGG Example. Canvas");
  if(app.init(600, 400, agg::window_resize))
  {
    return app.run();
  }
  return -1;
}