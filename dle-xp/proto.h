// Copyright (C) 1997 Bryan Aamot
int Query2Msg(LPCSTR lpszText,int option); // dlc.cpp
void auto_link_exit_to_reactor();
void define_wall(INT16 segnum, INT16 sidenum,INT16 wallnum,UINT8 type,INT8 clipnum,INT16 tmapnum);
void set_wall_textures(INT16 wallnum);
int DefineTexture(INT16 tmap_num,INT16 tmap_num2,UINT8 *bitmap_array,int x0, int y0);
int ReadTextureFromFile(INT16 texture_number,UINT8 *bitmap_array);
void free_texture_handles();
char *PaletteResource();
BITMAPINFO FAR *make_bitmap(void);
void set_object_data(INT8 type);
void delete_object();
bool copy_object(UINT8 new_type);
void size_line(segment *FAR seg,int point0,int point1,INT32 inc);
void rotate_vms_matrix(vms_matrix *orient,double angle,char axis);
void rotate_vms_vector(vms_vector *vector,double angle,char axis);
void update_cube_uv();
void rotate_selection(double angle, bool perpindicular);
double dround_off(double value, double round);
void free_texture_handles();
void renumber_matcen();
bool wall_is(int wallnum,UINT8 type,UINT8 flags,UINT8 keys);
void mark_cube(INT16 segnum);
void rotate_uv(double rotation_angle);
int round_int(int value, int round);
void adjoint_matrix(double A[3][3],double B[3][3]);
void square2quad_matrix(double A[3][3],POINT a[4]);
void multiply_matrix(double C[3][3], double A[3][3], double B[3][3]);
void scale_matrix(double A[3][3], double scale);
int ReadTextureFromFile(INT16 texture_number,UINT8 *bitmap_array);
void set_object_data(INT8 type);
bool copy_object(UINT8 new_type);
void free_texture_handles();
void set_wall_textures(INT16 wallnum);
char *PaletteResource();
bool copy_object(UINT8 new_type);
void auto_link_exit_to_reactor(); // control.cpp
int DefineTexture(INT16 tmap_num,INT16 tmap_num2,UINT8 *bitmap_array);
void free_texture_handles();
char *PaletteResource();
HWND MyFindWindow(HWND parent,LPSTR title);
void set_point(vms_vector *vert,APOINT *apoint);
void draw_line(HDC hdc,INT16 segnum,INT16 vert1,INT16 vert2);
void draw_spline(HDC hdc);
void fixtoa(char *str, FIX value);
HWND GetAppWindow();
void erase_tool_bar_help(int button);
void draw_tool_bar_help(int button);
void enable_children(INT16 segnum,INT16 recursion_level);
void center_on_cube();
void add_exit(INT16 type);
void add_door_trigger(INT16 wall_type,UINT16 wall_flags,UINT16 trigger_type);
bool get_opposing_side(INT16 *opp_segnum,INT16 *opp_sidenum,INT16 segnum, INT16 sidenum);



void make_mission_file(char *file_name,int custom_textures, int custom_robots);
void make_hog(char *rdl_file_name,char *hog_file_name, int make_mission_file);
void write_sub_file(FILE *hogfile,char *filename,char *name);
void dump_seg_info(FILE *file,INT16 segnum);
INT16 count_marked_segments();
INT16 read_seg_info(FILE *file);
void delete_seg(INT16 delete_segnum);
bool check_id(int type,int id);
double calculate_angle(INT16 vert0,INT16 vert1,INT16 vert2,INT16 vert3);
double calculate_flatness_ratio(INT16 segnum, INT16 sidenum);
double calculate_distance(vms_vector &v1,vms_vector &v2,vms_vector &v3);
void delete_object();
int align_textures(INT16 start_segment,INT16 start_side,INT16 only_child);
void unlink_child(INT16 segnum,INT16 sidenum);
void spin_point(vms_vector &point,double y_spin,double z_spin);
void spin_back_point(vms_vector &point,double y_spin,double z_spin);
int matching_side(int j);
void delete_seg(INT16 delete_segnum);
void define_verticies(INT16 new_verts[4]);
void polar_points(double *angle,double *radius,vms_vector *vertix, vms_vector *orgin,vms_vector *normal);
void rect_points(double angle,double radius,vms_vector *vertix,vms_vector *orgin,vms_vector *normal);
double coef(int n, int i);
long fact(int q);
double blend(int i, int n, double u);
void BezierFcn(vms_vector *pt, double u, int npts, vms_vector *p);
void calculate_center(vms_vector &center,INT16 segnum,INT16 sidenum);
void calculate_orthogonal_vector(vms_vector &c,INT16 segnum,INT16 sidenum);
double calculate_length(vms_vector &center1,vms_vector &center2);
void untwist_segment(INT16 segnum,INT16 sidenum);
double atan3(double y, double x);
void make_object(object FAR *obj, INT8 type, INT16 segnum);
void calculate_segment_center(vms_vector &pos,INT16 segnum);
void calculate_spline();
void fix_children();
int  ErrorMsg(LPCSTR lpszText);
int  QueryMsg(LPCSTR lpszText);
int  InfoMsg(LPCSTR lpszText);
int  DebugMsg(LPCSTR lpszText);
bool link_segments(INT16 segnum1,INT16 sidenum1,INT16 segnum2,INT16 sidenum2,FIX margin);
void delete_vertix(INT16 deleted_vertnum);
bool vertix_used(INT16 vertnum);
void delete_unused_vertices();
bool used_vertix(INT16 vertnum);
void allocate_memory();
void init_data();
void cleanup(INT16 error);
void update_display(HDC hDC);
void draw_segment_quick(HDC hDC,segment FAR *seg);
void draw_object(HDC hDC,INT16 i,INT16 clear_it);
void draw_highlight(HDC hDC,INT16 clear_it);
void draw_walls(HDC hDC);
void print_data();
char *htoa(INT16 value);
APOINT *set_vector(vms_vector *vector);
void set_matrix();
//segment FAR *calc_far_seg(segment FAR *base,INT32 offset);
void set_lines_to_draw();
void draw_partial_segment(HDC hDC,segment FAR *seg);
void draw_marked_segments(HDC hDC, INT16 clear_it);
void draw_segment(HDC hDC,INT16 segnum,INT16 sidenum, INT16 linenum, INT16 pointnum, INT16 clear_it);
void draw_segment_points(HDC hDC,segment FAR *seg);


/* segment.c prototypes */
move_marked_segments(INT16 x,INT16 y,INT16 z);
INT16 count_marked_segments();
INT16 count_segments();
INT16 first_free_segment();
void dump_seg_info(FILE *file,INT16 segnum);
INT16 read_seg_info(FILE *file,INT16 segnum);
void select_current_segment(HDC hDC,INT16 direction,INT16 cursor_x,INT16 cursor_y);
void get_next_segment(INT16 inc);
void get_next_special_segment(INT16 inc);
//void delete_segment(INT16 segnum);
//void mark_segment(INT16 segnum);
//void event_join_segments();
//void add_segment();
void jump_to_segment();
//void event_mark_segment();
//void event_delete_segment();
//void event_add_segment();

/* event.c prototypes */
//void get_key_event(char ch);
//void do_key_event(char ch, unsigned char ext_key, INT16 special);
char *htoa(INT16 value);
void get_mouse_event();
void jump_to_child(INT16 child);
void jump_to_object();
void move_on(char axis,INT32 inc);
void size_item(INT32 inc);
void spin_selection(double angle);
void rotate_vertix(vms_vector *vertix,vms_vector *point1,vms_vector *point2,double spin);
void wrap(INT16 *x,INT16 delta,INT16 min,INT16 max);
void block_cut();
void block_copy();
void block_paste();
INT16 count_marked_vertices();
INT16 count_vertices();
INT16 first_free_vertix();
INT16 read_seg_info(FILE *file,INT16 segnum);
//void unknown_key(char ch,char ext_key,char special);
//void event_play_macro();
//void event_toggle_macro();
//void event_load();
//void event_save();
//void event_quit();
//void event_other_current();
//void event_objects_toggle();
//void event_special_toggle();
//void event_walls_toggle();
//void event_lines_toggle();
//void event_prev_line();
//void event_next_line();
//void event_points_toggle();
//void event_zoom_out();
//void event_zoom_in();
//void event_edit_toggle();
//void event_quit();
//void event_next_cube();
//void event_prev_cube();
//void event_next_object();
//void event_prev_object();
void select_current_object(HDC hDC,INT16 cursor_x,INT16 cursor_y);
void event_add_object();
void event_delete_object();
//void event_next_point();
//void event_prev_point();
//void event_next_mode();
//void event_prev_mode();
//void event_home();
//void event_jump_to_prev_child();
//void event_jump_to_child();
//void event_new();
//void event_directory();

/* level.c prototypes */
//void error (char *str);
void ReadObject(object *obj,FILE *f,INT32 version);
//void Int3();
void ClearMineData();
INT16 load_level(char FAR *filename_passed);
INT32 read_INT32(FILE *load_file);
INT16 LoadMineDataCompiled(FILE *load_file);
INT16 LoadGameData(FILE *loadfile);
void cleanup(INT16 error);
INT32 read_INT32(FILE *load_file);
INT16 read_INT16(FILE *load_file);
INT8 read_INT8(FILE *load_file);
FIX read_FIX(FILE *load_file);
FIXANG read_FIXANG(FILE *load_file);
vms_matrix *read_matrix(vms_matrix *matrix,FILE *load_file);
vms_vector *read_vector(vms_vector *vector,FILE *load_file);
vms_angvec *read_angvec(vms_angvec *vector,FILE *load_file);
size_t _Cdecl cfread(void *ptr, size_t size, size_t n, FILE *stream);
//void pause();

/* save_lvl.c prototypes */
void WriteObject(object *obj,FILE *f,INT32 version);
INT16 save_level(char * filename_passed);
INT16 SaveMineDataCompiled(FILE *save_file);
INT16 SaveGameData(FILE *savefile);
INT32  write_INT32 (INT32 ,FILE *save_file);
INT16  write_INT16 (INT16 ,FILE *save_file);
INT8   write_INT8  (INT8  ,FILE *save_file);
FIX    write_FIX   (FIX   ,FILE *save_file);
FIXANG write_FIXANG(FIXANG,FILE *save_file);
vms_matrix *write_matrix(vms_matrix *matrix,FILE *save_file);
vms_vector *write_vector(vms_vector *vector,FILE *save_file);
vms_angvec *write_angvec(vms_angvec *vector,FILE *save_file);
size_t _Cdecl cfwrite(void *ptr, size_t size, size_t n, FILE *stream);


/* texture.c */
//void modify_wall_data();
//void event_modify_data();
//void modify_object_data();
//void modify_segment_data();
//void event_next_texture();
//void event_prev_texture();
//void event_next_texture2();
//void event_prev_texture2();
void show_texture(UINT16 texture_number,FILE *file,
                  INT16 x_offset,INT16 y_offset,INT16 mode);
void ReadPigTextureTable();
//void show_explosion();


/* help.c */
//void event_show_info();
//void help();
//void about();
//void show_file_message(char *mark);
//void tutorial();
INT16 read_message(char *message,char *filename,char *marker);
INT8 freadln(char *str, FILE *file);


/* wall.c */
//void event_add_wall();
//void event_delete_wall();
//void event_prev_side();
//void event_next_side();

