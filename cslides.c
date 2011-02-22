#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>
#include <strings.h>
#include <X11/Xlib.h>
#include <Imlib2.h>

/* 
 * a minimal slideshow viewer.
 * feed image file path stream to stdin and it will display each image for a second.
 * usage:  find . -name '*.jpg' | $0 [delay-in-seconds]
 */

struct img {
	Pixmap pixmap;
	int width;
	int height;
};
struct coord {
	unsigned int x;
	unsigned int y;
};

struct img load_img(Display *disp,
		    Window win,
		    Visual *vis,
		    Colormap cm,
		    int depth,
		    const char *imgfile)
{
	int x=0;
	int y=0;
	int target_size=480*360; /* xx take from command line */
	int width=-1;
	int height=-1;
	int source_size=-1;
	float factor=0;
	Imlib_Image iml;
	struct img img;

	imlib_context_set_display(disp);
	imlib_context_set_visual(vis);
	imlib_context_set_colormap(cm);

	iml = imlib_load_image(imgfile);
	assert(iml);
	imlib_context_set_image(iml);

	width = imlib_image_get_width();
	height = imlib_image_get_height();
	source_size=width*height;

	factor=sqrt(((float)target_size)/source_size);
	if (factor<1) {
		img.width=(int)width*factor;
		img.height=(int)height*factor;
	} else {
		img.width=width;
		img.height=height;
	}
	img.pixmap = XCreatePixmap(disp, win, img.width, img.height, depth);
	imlib_context_set_drawable(img.pixmap);
	/* 
	 * this is very implicit; writing image loaded above (iml), to pixmap created and set here.
	 */
	imlib_render_image_on_drawable_at_size(x, y, img.width, img.height); 

	return img;
}

void finish_img(struct img img, Display *disp)
{
				/* 
				 * assuming that the current image is
				 * the one created with
				 * imlib_load_image() and no
				 * imlib_context_set_image() has been
				 * called since.
				 * Perhaps Imlib_Image should be kept on img so it can be set here..
				 */
	imlib_free_image();
	XFreePixmap(disp, img.pixmap);
}

int main(int argc, char *argv[]) 
{
	XEvent ev;
	Window              win;
	Display            *disp;
	Visual             *vis;
	Colormap            cm;
	int                 depth;
	struct img img;
	int delay = 1;
	struct coord window_placement={ 400, 200 }; /* todo: center it */

	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	if (argc>1) {
		delay=atoi(argv[1]);
	}

	disp = XOpenDisplay(NULL);
	vis = DefaultVisual(disp, DefaultScreen(disp));
	depth = DefaultDepth(disp, DefaultScreen(disp));
	cm = DefaultColormap(disp, DefaultScreen(disp));
				/* x,y,w,h,border_width,border,background */
	win = XCreateSimpleWindow(disp, DefaultRootWindow(disp), 0, 0, 10, 10, 0, 0, 0);

	XSelectInput(disp, win, StructureNotifyMask|ExposureMask);
	XMapWindow(disp, win);
				/* 
				 * read path stream from stdin
				 */
	while ((read = getline(&line, &len, stdin)) != -1) {

		char *found=index(line, '\n');
		assert(found);	/* xx expecting newline-terminated lines */
		*found='\0';
				/* echo the input */
		printf("%s\n", line);

		img=load_img(disp, win, vis, cm, depth, line);

		XSetWindowBackgroundPixmap(disp, win, img.pixmap);
				/* todo: resize window to a consistant size. */
		XMoveResizeWindow(disp, win, window_placement.x, window_placement.y, img.width, img.height);

				/* wait till the request takes effect */
		for (;;) {
			XNextEvent(disp, &ev); 
			if (ev.type==MapNotify)
				break;
			else if (ev.type==Expose)
				break;
		}
				/* xx or until user clicks on the img.. */
		sleep(delay);
				/* 
				 * clean up
				 */
		finish_img(img, disp);

		assert(line);
		free(line);
		line=NULL;
	}

	// TODO clean up

	return 0;
}
