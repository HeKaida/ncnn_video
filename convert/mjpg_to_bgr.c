#include <string.h>
#include <stdlib.h>

#include "config.h"
#include "video_manager.h"
#include "convert_manager.h"
#include "jpeglib.h"


#define JPEG_MEM_SRC_SUPPORTED 1


static int mjpeg_to_bgr(PT_VideoDevice ptVideoDeviceIn, PT_VideoConvert_Buf ptVideoConvert_Buf)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	memset(&cinfo, 0, sizeof(struct jpeg_decompress_struct));
	memset(&jerr, 0, sizeof(struct jpeg_error_mgr));

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	jpeg_mem_src(&cinfo, ptVideoDeviceIn->tVideoBuf.addr, ptVideoDeviceIn->tVideoBuf.bytesused);

	jpeg_read_header(&cinfo, TRUE);
	cinfo.out_color_space = JCS_EXT_BGR;

	jpeg_start_decompress(&cinfo);
	ptVideoConvert_Buf->iWidth = cinfo.output_width;
	ptVideoConvert_Buf->iHeight = cinfo.output_height;
	int row_stride = cinfo.output_width * cinfo.output_components;

	ptVideoConvert_Buf->rgb_out = calloc(1, cinfo.output_width * cinfo.output_height * 3);
	unsigned char* buffer = ptVideoConvert_Buf->rgb_out;


	while (cinfo.output_scanline < cinfo.output_height) 
	{
        unsigned char* row_ptr[1];
        row_ptr[0] = buffer + (cinfo.output_scanline) * row_stride;
        jpeg_read_scanlines(&cinfo, row_ptr, 1);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    
    return 0;
}


static T_VideoConvert_Opr g_tVideoConvert_Opr = 
{
	.name    = "mjpg2bgr",
	
	.Convert = mjpeg_to_bgr,
};


int Mjpeg2bgrInit(void)
{
	return RegisterConvertOpr(&g_tVideoConvert_Opr);
}
