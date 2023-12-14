CXX = g++
CXXFLAGS = -std=c++17

SRC_DIR = src
OBJ_DIR = obj


OUT_COMP = company.out
OUT_BUILD = building.out
OUT_OFFICE = office.out
OUT_RESOURCE = resource.out

VPATH = $(SRC_DIR)


all: $(OUT_COMP) $(OUT_BUILD) $(OUT_OFFICE) $(OUT_RESOURCE)

$(OUT_COMP): $(OBJ_DIR)/company.o $(OBJ_DIR)/logger.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OUT_BUILD): $(OBJ_DIR)/building.o $(OBJ_DIR)/logger.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OUT_OFFICE): $(OBJ_DIR)/office.o $(OBJ_DIR)/logger.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OUT_RESOURCE): $(OBJ_DIR)/resource.o $(OBJ_DIR)/logger.o
	$(CXX) $(CXXFLAGS) $^ -o $@


$(OBJ_DIR)/company.o: $(SRC_DIR)/main.cpp $(SRC_DIR)/logger.hpp $(SRC_DIR)/color_print.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/building.o: $(SRC_DIR)/building.cpp $(SRC_DIR)/logger.hpp $(SRC_DIR)/color_print.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/office.o: $(SRC_DIR)/office.cpp $(SRC_DIR)/logger.hpp $(SRC_DIR)/color_print.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/resource.o: $(SRC_DIR)/resource.cpp $(SRC_DIR)/logger.hpp $(SRC_DIR)/color_print.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


$(OBJ_DIR)/logger.o: $(SRC_DIR)/logger.cpp $(SRC_DIR)/logger.hpp $(SRC_DIR)/color_print.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)



.PHONY: all clean

clean:
	rm -f $(OBJ_DIR)/*.o ./*.out
