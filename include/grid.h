template <class T>
class Grid {
 public:
  /**
   * Constructor.
   * @param w width
   * @param h height
   */
  Grid(unsigned int w, unsigned int h) {
    this->w = w;
    this->h = h;
    int size = this->size();
    quantities = new T[w*h];
  };
  
  ~Grid(){
    delete[] quantities;
  }

  /**
   * Size.
   * @returns the total number of cells
   */
  unsigned int size() const{
    return w*h;
  };


  /**
   * Get value of the stored quantity.
   * @param i, the position along the x axis (w)
   * @param j, the position along the y axis (h)
   */
  T get(unsigned int i, unsigned int j) const{
    return quantities[j*w + i];
  };

  
  
  /**
   * Set value of the stored quantity.
   */
  void set(unsigned int i, unsigned int j, T value) {
    quantities[j*w + i] = value;
  };


 private:
  unsigned int w, h;
  T *quantities;
  
};
