im_p = "/Users/nick/Downloads/00003652_0002_15 copy.jpeg";
im = imread( im_p );

%%

im_bw = double(rgb2gray(im)) ./ 255;
[gx, gy] = gradient( im_bw );

figure(1); clf;
subplot( 1, 3, 1 );
imagesc( gx );
subplot( 1, 3, 2 );
imagesc( gy ); colorbar;
subplot( 1, 3, 3 );
imshow( im_bw );

t = 0.05;
[gx, gy] = gradient( im_bw );
is_edge = abs(gx) > t | abs(gy) > t;

figure(2); clf;
bin_is_edge = uint8( 255 * is_edge );
imshow( bin_is_edge );

if ( 1 )
  dst_p = fullfile( fileparts(im_p), "edge_im.png" );
  imwrite( bin_is_edge, dst_p );
end

%%

%{
1. determine the edges in the image: binarize the image so that 1 is an
   edge
2. for every pixel in the image, compute the vector to the nearest
   edge
%}

% 1.
t = 0.05;
[gx, gy] = gradient( im_bw );
is_edge = abs(gx) > t | abs(gy) > t;

figure(2); clf;
imagesc( is_edge );

edge_dir_func = @(im, si, sj) nearest_edge_dir_beam( im, si, sj, [] );
% edge_dir_func = @nearest_edge_dir;

% 2.
vec_im = zeros( size(im, 1), size(im, 2), 3 );
for i = 0:size(is_edge, 1)-1
  fprintf( '\n %d of %d', i+1, size(is_edge, 1) );
  for j = 0:size(is_edge, 2)-1
    [tf, v] = edge_dir_func( is_edge, i, j );
    if ( tf )
      vec_im(i+1, j+1, 1:2) = v;
    end
  end
end

%%

manip = @to_angle;
di = manip( vec_im );

figure(3); clf;
subplot( 1, 2, 1 );
imagesc( manip(g_truth_vec_im) );
subplot( 1, 2, 2 );
imagesc( manip(vec_im) );
colorbar;

%%

function th = to_angle(dir_im)
th = atan2( dir_im(:, :, 2), dir_im(:, :, 1) ) + pi;
th = min( 1, max(0, th/pi) );
end

function [tf, gv] = nearest_edge_dir_beam(edge_im, si, sj, max_stp)

if ( isempty(max_stp) )
  max_stp = max( size(edge_im) );
end

max_stp = max( 0, min(max_stp, max(size(edge_im))) );

s = [ si, sj ];
gv = [];
stp = 0;

while ( stp < max_stp )
  off = 1 + stp;
  stp = stp + 1;

  % init indices
  i0 = max( 0, si - off );
  i1 = min( size(edge_im, 1), si + off + 1 );
  j0 = max( 0, sj - off );
  j1 = min( size(edge_im, 2), sj + off + 1 );

  % init iteration
  dmin = inf;
  v = [];
  % top/bot strips
  for j = j0:j1-1
    % top
    ei = si - off;
    if ( ei >= 0 && edge_im(ei+1, j+1) )
      vt = [ ei, j ] - s;
      if ( norm(vt) < dmin )
        dmin = norm( vt );
        v = vt;
      end
    end
    % bot
    ei = si + off;
    if ( ei < size(edge_im, 1) && edge_im(ei+1, j+1) )
      vt = [ ei, j ] - s;
      if ( norm(vt) < dmin )
        dmin = norm( vt );
        v = vt;
      end
    end
  end
  % l/r strips
  for i = i0:i1-1
    % left
    ej = sj - off;
    if ( ej >= 0 && edge_im(i+1, ej+1) )
      vt = [ i, ej ] - s;
      if ( norm(vt) < dmin )
        dmin = norm( vt );
        v = vt;
      end
    end
    % right
    ej = sj + off;
    if ( ej < size(edge_im, 2) && edge_im(i+1, ej+1) )
      vt = [ i, ej ] - s;
      if ( norm(vt) < dmin )
        dmin = norm( vt );
        v = vt;
      end
    end
  end

  if ( ~isempty(v) )
    gv = v;
    break
  end
end

tf = ~isempty( gv );

end

%%

function [tf, v] = nearest_edge_dir(edge_im, si, sj)

dmin = inf;
di = 0;
dj = 0;
s = [ si, sj ];
v = [];

for i = 0:size(edge_im, 1)-1
  for j = 0:size(edge_im, 2)-1
    if ( edge_im(i+1, j+1) && i ~= si && j ~= sj )
      d = norm( [i, j] - s );
      if ( d < dmin )
        dmin = d;
        di = i;
        dj = j;
      end
    end
  end
end

tf = dmin ~= inf;
if ( tf )
  v = [ di, dj ] - s;
end

end