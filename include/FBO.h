#include <iostream>
#include <common/Texture2D.h>

class FBO {
public:
  FBO(GLuint w, GLuint h, GLenum tb) {
    this->w = w;
    this->h = h;
    this->texBucket = tb;

    // create new framebuffer
    glGenFramebuffers(1, &framebufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferId);

    // create texture
    texture = new Texture2D(w, h);

    // init texture to all 0's
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, w, h, 0, GL_RGB, GL_FLOAT, 0);

    // filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // init config
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture->getId(), 0);
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers);

    // fail check
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      std::cout << "failed to init FBO" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  };

  ~FBO() {
    glDeleteFramebuffers(1, &framebufferId);
    delete texture;
  };

  void activateWrite() {
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferId);
  };

  static void deactivateFramebuffers() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  };

  GLuint getTextureId() {
    return texture->getId();
  };

  Texture2D const *const getTexture() const {
    return texture;
  };

  GLenum getTextureBucket() {
    return texBucket;
  }

private:
  GLuint w, h;
  GLuint framebufferId;
  Texture2D *texture;
  GLenum texBucket;
};