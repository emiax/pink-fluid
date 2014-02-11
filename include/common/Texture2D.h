#pragma once
class Texture2D{
public:
  Texture2D(GLuint w, GLuint h){
    this->w = w; 
    this->h = h;
    d = 4;
    glGenTextures(1, &textureID);

    glBindTexture(GL_TEXTURE_2D, textureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);	
    data = new GLfloat[h*w*d];
    for (int i = 0; i < h; ++i) {
      for (int j = 0; j < w; ++j){
        data[indexTranslation(i,j,0)] = 0.0f;
        data[indexTranslation(i,j,0)] = 0.0f;
        data[indexTranslation(i,j,0)] = 0.0f;
        data[indexTranslation(i,j,0)] = 1.0f;
      }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
  ~Texture2D(){
    glDeleteTextures(1, &textureID);
  }
  GLfloat get(GLuint i, GLuint j, GLuint k){
    return data[indexTranslation(i,j,k)];
  }
  void set(GLuint i, GLuint j, GLuint k, GLfloat v){
    data[indexTranslation(i,j,k)] = v;
  }
  operator GLuint(){
    return textureID;
    delete[] data;
  }
  void operator()(GLenum texture) { 
    glActiveTexture(texture);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, data);
  }
  
private:
  GLuint w, h,d;
  GLuint textureID; 
  GLfloat *data;
  GLuint indexTranslation(GLuint i, GLuint j, GLuint k){
    return i*w*d + j*d + k;
  }
};
