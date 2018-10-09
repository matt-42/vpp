
int main()
{
  image2d<vuchar3> rgb(100,100);
  //hehe
  auto red = image_view(rgb, [] (vint2 p) { return rgb(p)[0]; });
  auto sub_red = image_view(rgb.subimage(vint2(0,0), vint2(10,10)),
                            [] (vint2 p) { return rgb(p)[0]; });
  
}
