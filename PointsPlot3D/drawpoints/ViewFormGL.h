///////////////////////////////////////////////////////////////////////////////
// ViewFormGL.h
// ============
// View component of OpenGL dialog window
//openGL对话框的视图组件
//  AUTHOR: Fang Liang (fangliang1313@gmail.com)
// CREATED: 2018-12-20
// UPDATED: 2018-12-20
///////////////////////////////////////////////////////////////////////////////

#ifndef VIEW_FORM_GL_H
#define VIEW_FORM_GL_H

#include <windows.h>
#include "Controls.h"
#include "ModelGL.h"
using namespace std;


namespace Win
{
    class ViewFormGL
    {
    public:
        ViewFormGL(ModelGL* model);
        ~ViewFormGL();

        void initControls(HWND handle);         // init all controls
        void updateTrackbars(HWND handle, int position);
        void setViewMatrix(float x, float y, float z, float p, float h, float r);  //设置视图矩阵
        void setModelMatrix(float x, float y, float z, float rx, float ry, float rz);  //设置模型矩阵
        void updateMatrices();
		int getComboSelect(int ComboBoxID);
		void setEditText(wstring filename);
		void setEditImage1Text(string filename);
		void setEditImage2Text(string filename);
		string getEditText(HWND handle);
		float *getColorSelect();
		float *getBackColorSelect();

    protected:

    private:
        ModelGL* model;
        HWND parentHandle;

        // controls for view section (camera)
        Win::Button   buttonResetView;
        Win::TextBox  textViewPosX;
        Win::TextBox  textViewPosY;
        Win::TextBox  textViewPosZ;
        Win::TextBox  textViewRotX;
        Win::TextBox  textViewRotY;
        Win::TextBox  textViewRotZ;
        Win::Trackbar sliderViewPosX;
        Win::Trackbar sliderViewPosY;
        Win::Trackbar sliderViewPosZ;
        Win::Trackbar sliderViewRotX;
        Win::Trackbar sliderViewRotY;
        Win::Trackbar sliderViewRotZ;
        Win::TextBox  textViewGL;

        // controls for model section
        Win::Button   buttonResetModel;
        Win::TextBox  textModelPosX;
        Win::TextBox  textModelPosY;
        Win::TextBox  textModelPosZ;
        Win::TextBox  textModelRotX;
        Win::TextBox  textModelRotY;
        Win::TextBox  textModelRotZ;
        Win::Trackbar sliderModelPosX;
        Win::Trackbar sliderModelPosY;
        Win::Trackbar sliderModelPosZ;
        Win::Trackbar sliderModelRotX;
        Win::Trackbar sliderModelRotY;
        Win::Trackbar sliderModelRotZ;
        Win::TextBox  textModelGL;

		//  OpenGL绘图区控件
		Win::Button   buttonOpenFile;
		Win::Button   buttonReDraw;
		Win::ComboBox comboboxCoordinateX;
		Win::ComboBox comboboxCoordinateY;
		Win::ComboBox comboboxCoordinateZ;
		Win::ComboBox comboboxColor;
		Win::ComboBox comboboxBackColor;
		Win::EditBox  editboxFileName;
		Win::EditBox  editboxImage1Name;
		Win::EditBox  editboxImage2Name;
        Win::Button buttonAbout;
		Win::RadioButton setCamera;
		Win::RadioButton hideCamera;

        // matrices
        Win::TextBox   mv[16];          // view matrix
        Win::TextBox   mm[16];          // model matrix
        Win::TextBox   mmv[16];         // modelview matrix
    };
}

#endif
