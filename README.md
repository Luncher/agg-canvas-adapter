## agg canvas adapter

### Draw Line

```javascript
  context->begin_path();
  context->set_line_width(8);
  context->set_stroke_color(0.1, 0.1, 0.5, 0.8);
  context->move_to(30, 30);
  context->set_line_cap("round");
  context->line_to(100, 100);
  context->stroke();
```

### Draw Rectangle

```javascript
  context->set_line_width(2);
  context->rect(20, 20, 50, 50, 3);
  context->stroke();  
  context->fill_rect(220, 220, 60, 60);
  context->stroke_rect(30, 100, 60, 60);
  context->clear_rect(230, 230, 40, 40);

```

### Transform

```javascript
  context->stroke_rect(450,5,25,15);
  context->scale(2,2);
  context->stroke_rect(450,5,25,15);
```

---

## [MIT License](https://opensource.org/licenses/MIT)