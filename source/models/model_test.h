// Alias OBJ Model File reduced to integers

const Vertex_t g_model_verts[] = {
		{80,80,-80}
	,	{-80,80,-80}
	, {-80,-80,-80}
	, {80,-80,-80}
	, {80,80,80}
	, {-80,80,80}
	, {-80,-80,80}
	, {80,-80,80}
	
	, {60,80,80}
	, {60,60,80}
	, {80,60,80}
	, {80,60,60}
	, {80,80,60}
	, {60,80,60}
	, {60,60,60}
};
const Line_t g_model_lines[] = {
		{1,2}
	, {2,3}
	, {3,4}
	, {4,1}
	, {5,6}
	, {6,7}
	, {7,8}
	, {8,5}
	, {1,5}
	, {2,6}
	, {3,7}
	, {4,8}

	, {9,10}
	, {10,11}
	, {11,12}
	, {12,13}
	, {13,14}
	, {14,9}
	, {10,15}
	, {12,15}
	, {14,15}
};

const Model_t g_model = {
		g_model_verts
	, g_model_lines
	, 15
	, 21
};

// This is what the 3d routine uses to store it's modified vertexes
Point_t g_pointBuffer[15];