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
    for (unsigned int i = 0; i < w; ++i) {
      for (unsigned int j = 0; j < h; ++j){
        data[indexTranslation(i,j,0)] = 0.0f;
        data[indexTranslation(i,j,1)] = 0.0f;
        data[indexTranslation(i,j,2)] = 0.0f;
        data[indexTranslation(i,j,3)] = 1.0f;
      }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // enforce no texture wrap
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
  ~Texture2D(){
    glDeleteTextures(1, &textureID);
    delete[] data;
  }
  const GLfloat get(GLuint i, GLuint j, GLuint k) const{
    return data[indexTranslation(i,j,k)];
  }
  void set(GLuint i, GLuint j, GLuint k, GLfloat v){
    data[indexTranslation(i,j,k)] = v;
  }
  operator const GLuint() const {
    return textureID;
  }
  void operator()(GLenum texture) { 
    glActiveTexture(texture);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, data);
  }
  void download() {
    //std::cout << this->w << std::endl;
    //    std::cout << this->h << std::endl;
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glReadPixels(0, 0, this->w, this->h, GL_RGBA, GL_FLOAT, data);
    //glReadPixels(0, 0, 128, 128, GL_RGBA, GL_FLOAT, data);
    /*    for (int i = 0; i < this->w*this->h; i++) {
      std::cout << data[i] << " ";
      }*/

  }
  int getWidth() {
    return w;
  }
  int getHeight() {
    return h;
  }

private:
  GLuint w, h, d;
  GLuint textureID; 
  GLfloat *data;
  GLuint indexTranslation(GLuint i, GLuint j, GLuint k) const {
    return j*w*d + i*d + k;
  }
};
